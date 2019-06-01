#include "Calculate.h"
#include "Config.h"
#include "Log.h"
#include "Response.h"
#include "Optimize.h"
#include "Filter.h"

#include <numeric>
#include <cmath>
#include <algorithm>

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

double interpolate( vector<double> &xData, vector<double> &yData, double x, bool extrapolate )
{
	int size = xData.size();

	int i = 0;                                                                  // find left end of interval for interpolation
	if ( x >= xData[size - 2] )                                                 // special case: beyond right end
	{
		i = size - 2;
	}
	else
	{
		while ( x > xData[i+1] ) i++;
	}
	double xL = xData[i], yL = yData[i], xR = xData[i+1], yR = yData[i+1];      // points on either side (unless beyond ends)
	if ( !extrapolate )                                                         // if beyond ends of array and not extrapolating
	{
		if ( x < xL ) yR = yL;
		if ( x > xR ) yL = yR;
	}

	double dydx = ( yR - yL ) / ( xR - xL );                                    // gradient

	return yL + dydx * ( x - xL );                                              // linear interpolation
}
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

static Response get_loudness(const Response &current, double spl) {
    const double f[] = { 20, 25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500 };
    const double af[] = { 0.532, 0.506, 0.480, 0.455, 0.432, 0.409, 0.387, 0.367, 0.349, 0.330, 0.315, 0.301, 0.288, 0.276, 0.267, 0.259, 0.253, 0.250, 0.246, 0.244, 0.243, 0.243, 0.243, 0.242, 0.242, 0.245, 0.254, 0.271, 0.301 };
    const double Lu[] = { -31.6, -27.2, -23.0, -19.1, -15.9, -13.0, -10.3, -8.1, -6.2, -4.5, -3.1, -2.0, -1.1, -0.4, 0.0, 0.3, 0.5, 0.0, -2.7, -4.1, -1.0, 1.7, 2.5, 1.2, -2.1, -7.1, -11.2, -10.7, -3.1 };
    const double Tf[] = { 78.5, 68.7, 59.5, 51.1, 44.0, 37.5, 31.5, 26.5, 22.1, 17.9, 14.4, 11.4, 8.6, 6.2, 4.4, 3.0, 2.2, 2.4, 3.5, 1.7, -1.3, -4.2, -6.0, -5.4, -1.5, 6.0, 12.6, 13.9, 12.3 };

    double Ln = spl;
    vector<double> freqs;

    for (size_t i = 0; i < sizeof(f) / sizeof(double); i++) {
        double Af = 4.47e-3 * (pow(10.0, 0.025 * Ln) - 1.15) +
                    pow(0.4 * pow(10.0, (((Tf[i]+Lu[i])/10)-9)), af[i]);
        double Lp = (10 / af[i]) * log10(Af) - Lu[i] + 94;

        /* Remove offset */
        Lp -= spl;
        Lp = -Lp;

        //cout << "For SPL " << spl << " and freq " << f[i] << " db " << Lp << endl;
        freqs.push_back(Lp);
    }

    /* Interpolate */
    auto output = current;
    auto& o_freqs = output.freqs_;
    auto& dbs = output.gains_;

    vector<double> l_freqs(f, f + sizeof(f) / sizeof(double));

    for (size_t i = 0; i < o_freqs.size(); i++)
        dbs.at(i) = interpolate(l_freqs, freqs, o_freqs.at(i), false);

    return output;
}

static Response apply_loudness(const Response &current, double monitor, double playback) {
    auto monitor_response = get_loudness(current, monitor);
    auto playback_response = get_loudness(current, playback);
    auto target = current;

    for (size_t i = 0; i < monitor_response.freqs_.size(); i++) {
        target.gains_.at(i) += monitor_response.gains_.at(i) - playback_response.gains_.at(i);
    }

    return target;
}

static Response calculate_target(const Response& current) {
    auto target = current;

    // Insert house targets here, etc.
    // Also compute valid auto target ranges
    auto target_level = auto_target_level(current);

    if (Config::has(KEY_TARGET_SPL)) {
        target_level = Config::get<double>(KEY_TARGET_SPL, target_level);
    }

    fill(target.gains_.begin(), target.gains_.end(), target_level);
    Log(DEBUG) << "Set base target level to " << target_level << endl;

    bool in_room = Config::get<bool>(KEY_HC_SLOPE_ENABLE, false);
    bool loudness = Config::get<bool>(KEY_HC_LOUDNESS_ENABLE, false);
    bool shelf = Config::get<bool>(KEY_HC_SHELF_ENABLE, false);

    if (in_room) {
        Log(DEBUG) << "Adding house curve for in-room response by " << Config::get<double>(KEY_HC_SLOPE_PER_OCTAVE, 1) << " dB per octave\n";
        double max_boost = Config::get<double>(KEY_HC_SLOPE_PER_OCTAVE, 1) * 10;
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
    }

    if (loudness) {
        auto monitor = Config::get<double>(KEY_HC_LOUDNESS_MONITOR, 75);
        auto playback = Config::get<double>(KEY_HC_LOUDNESS_PLAYBACK, 75);
        Log(DEBUG) << "Adding house curve for loudness with monitor " << monitor << " dB and playback " << playback << " dB\n";
        target = apply_loudness(target, monitor, playback);
    }

    if (shelf) {
        auto bass_freq = Config::get<double>(KEY_HC_SHELF_BASS_FREQ, 120);
        auto bass_gain = Config::get<double>(KEY_HC_SHELF_BASS_GAIN, 0);
        auto treble_freq = Config::get<double>(KEY_HC_SHELF_TREBLE_FREQ, 8000);
        auto treble_gain = Config::get<double>(KEY_HC_SHELF_TREBLE_GAIN, 0);
        Log(DEBUG) << "Adding house curve for shelf with bass " << bass_freq << ":" << bass_gain << " and treble " << treble_freq << ":" << treble_gain << endl;
        const double SHELF_Q = 1 / sqrt(2); // 0.7071
        Filter bass(bass_freq, SHELF_Q, 0, 0, VALUE_FILTER_LOW_SHELF);
        Filter treble(treble_freq, SHELF_Q, 0, 0, VALUE_FILTER_HIGH_SHELF);
        bass.apply(target, bass_gain);
        treble.apply(target, treble_gain);
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
    if (Config::has(KEY_TARGET_LOW_LIMIT) && Config::has(KEY_TARGET_HIGH_LIMIT)) {
        // Forced limits
        Config::set(KEY_LOW_LIMIT, Config::get<string>(KEY_TARGET_LOW_LIMIT, "20"));
        Config::set(KEY_HIGH_LIMIT, Config::get<string>(KEY_TARGET_HIGH_LIMIT, "20000"));
    } else {
        calculate_limits(log_scaled, target);
    }

    Log(DEBUG) << "Set limits low " << Config::get<double>(KEY_LOW_LIMIT, 20)
               << " and high " << Config::get<double>(KEY_HIGH_LIMIT, 20000)
               << endl;

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