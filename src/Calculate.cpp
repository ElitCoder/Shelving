#include "Calculate.h"
#include "Config.h"
#include "Log.h"
#include "Response.h"
#include "Optimize.h"

#include <numeric>
#include <cmath>

using namespace std;

// From SO
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

static Response calculate_target(const Response& current) {
    auto target = current;

    fill(target.gains_.begin(), target.gains_.end(), 80);
    return target;
}

static bool calculate_filters(const Response& response) {
    // Calculate target
    auto target = calculate_target(response);

    // Go through each filter and optimize
    Optimize::optimize(response, target);

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