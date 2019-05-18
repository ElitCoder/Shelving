#include "Calculate.h"
#include "Config.h"
#include "Log.h"
#include "Response.h"
#include "Optimize.h"

#include <numeric>
#include <cmath>
#include<algorithm>

using namespace std;

// BEGIN From SO
template <typename T>
struct normalize {
    T operator()(T initial, T value) {
        return initial + pow(value - mean, 2);
    }
    T mean;
};

double get_sd(const vector<double>& values) {
    double sum = std::accumulate(values.begin(), values.end(), 0);
    double mean = sum / values.size();
    double var = std::accumulate(values.begin(), values.end(), 0,
                                 normalize<double>{ mean });
    return sqrt(var);
}

template<typename T = double>
class Logspace {
private:
    T curValue, base, step;

public:
    Logspace(T first, T last, int num, T base = 10.0) : curValue(first), base(base){
       step = (last - first)/(num-1);
    }

    T operator()() {
        T retval = pow(base, curValue);
        curValue += step;
        return retval;
    }
};
// END FROM SO

static double auto_target_level(const Response& target) {
    // Important SPL info is in the range of 500 - 2000 Hz
    const int VITAL_SPL_LEVEL_LOW = 500;
    const int VITAL_SPL_LEVEL_HIGH = 2000;

    // Set target SPL level from there
    size_t start = 0, end = target.freqs_.size() - 1;
    bool set_start = false, set_end = false;
    for (size_t i = 0; i < target.freqs_.size(); i++) {
        if (target.freqs_.at(i) >= VITAL_SPL_LEVEL_LOW && !set_start) {
            start = i;
            set_start = true;
            Log(DEBUG) << "Set end auto target START i to " << i << endl;
        }
        if (target.freqs_.at(i) > VITAL_SPL_LEVEL_HIGH && !set_end) {
            end = i - 1;
            set_end = true;
            Log(DEBUG) << "Set end auto target END i to " << i << endl;
        }
    }

    if (!set_start || !set_end) {
        Log(WARN) << "Failed to find start or end of target curve, will return average of whole curve\n";
    }

    // Average SPL level there
    double spl_average = accumulate(target.gains_.begin() + start, target.gains_.begin() + end, 0) / (double)(end - start);
    Log(DEBUG) << "Found SPL average to be " << spl_average << " dB\n";

    return spl_average;
}

static Response convert_linear_to_log(const Response& response) {
    //const int START = 20; // Lower hearing threshold, should be overridable
    //const int END = 20000; // Upper hearing threshold, should be overridable
    // In log representation
    const double START_LOG = 1.3;
    const double END_LOG = 4.3;

    // TODO: Perhaps add optimization limits here?
    vector<double> log_freqs;
    int num = Config::get<int>(KEY_DATA_POINTS, 1000); // Amount of points, should be overridable for accuracy
    generate_n(back_inserter(log_freqs), num, Logspace<>(START_LOG, END_LOG, num));

    // Map new frequencies to old gains
    vector<double> log_gains(log_freqs.size(), 0);
    #pragma omp parallel for
    for (size_t i = 0; i < log_freqs.size(); i++) {
        auto& log_freq = log_freqs.at(i);
        // TODO: Add some averaging instead
        int closest_index = -1;
        double closest;

        for (size_t j = 0; j < response.freqs_.size(); j++) {
            double distance = abs(response.freqs_.at(j) - log_freq);
            if (closest_index < 0 || distance < closest) {
                closest_index = j;
                closest = distance;
            }
        }

        if (closest_index < 0) {
            Log(WARN) << "Could not find fitting gain to frequency " << log_freq << endl;
            continue;
        }

        log_gains.at(i) = response.gains_.at(closest_index);
    }

    Response log_response;
    log_response.freqs_ = log_freqs;
    log_response.gains_ = log_gains;

    return log_response;
}

static Response calculate_target(const Response& current) {
    auto target = current;

    // Insert house targets here, etc.
    // Also compute valid auto target ranges
    auto target_level = auto_target_level(current);

    if (Config::has(KEY_TARGET_SPL)) {
        target_level = Config::get<double>(KEY_TARGET_SPL, target_level);
    }

    // Clean later
    double max_boost = 10;
    /*
        i/20 = 0
        20 / 20 = 1
    */

    fill(target.gains_.begin(), target.gains_.end(), target_level);

    Log(DEBUG) << "Set base target level to " << target_level << endl;

    //auto& log_freqs = target.freqs_;
    auto& log_gains = target.gains_;

    #pragma omp parallel for
    for (size_t i = 0; i < log_gains.size(); i++) {
        double sum = ((double)i / (double)log_gains.size()) * -max_boost;
        //Log(DEBUG) << "Adding " << sum << " for frequency " << log_freqs.at(i) << endl;
        log_gains.at(i) += sum;
    }

    #pragma omp parallel for
    for (size_t i = 0; i < log_gains.size(); i++) {
        auto& gain = log_gains.at(i);
        gain += max_boost / 2;
    }

    return target;
}

static void calculate_limits(const Response& response, const Response& target) {
    int start = 0;
    int end = target.freqs_.size() - 1;

    for (; start < (int)response.gains_.size(); start++) {
        if (response.gains_.at(start) >= target.gains_.at(start)) {
            break;
        }
    }

    for (; end >= 0; end--) {
        if (response.gains_.at(end) >= target.gains_.at(end)) {
            break;
        }
    }

    if (start > (int)response.freqs_.size() - 1) {
        start = response.freqs_.size() -1;
    }

    if (end < 0) {
        end = 0;
    }

    if (start > end) {
        Log(WARN) << "Target SPL set too high!\n";
    }

    // Set Config
    Log(DEBUG) << "Calculated limits low " << response.freqs_.at(start) << " and high " << response.freqs_.at(end) << endl;
    Config::set(KEY_LOW_LIMIT, to_string(response.freqs_.at(start)));
    Config::set(KEY_HIGH_LIMIT, to_string(response.freqs_.at(end)));
}

static bool calculate_filters(const Response& response) {
    // Convert input data to a log-spaced representation since that represents the human hearing better than a linear-spaced vector
    auto log_scaled = convert_linear_to_log(response);

    // Calculate target
    auto target = calculate_target(log_scaled);

#if 0
    Log(DEBUG) << "ACTUAL\n";
    log_scaled.print();
    Log(DEBUG) << "TARGET\n";
    target.print();
#endif

    // Calculate limits
    calculate_limits(log_scaled, target);

    // Go through each filter and optimize
    Optimize::optimize(log_scaled, target);

    return true;
}

bool calculate() {
    // By this point the config should be populated and we can just calculate
    // everything we need
    auto input_type = Config::get<string>(KEY_INPUT_TYPE, "");
    auto input = Config::get<string>(KEY_INPUT, "");

    if (input_type == VALUE_INPUT_TYPE_FR) {
        // Calculate optimal filter settings for specified FR
        auto response = Response::parse(input);
        return calculate_filters(response);
    } else if (input_type == VALUE_INPUT_TYPE_RAW_PINK) {
        Log(ERR) << "Not implemented\n";
        return false;
    } else if (input_type == VALUE_INPUT_TYPE_RAW_WHITE) {
        Log(ERR) << "Not implemented\n";
        return false;
    } else {
        Log(ERR) << "Unknown input type\n";
        return false;
    }

    return true;
}