#include "Filter.h"
#include "Log.h"
#include "Config.h"

#include <cmath>

using namespace std;

Filter::Filter(double freq, double q, double range_low, double range_high,
               const string& type) :
        freq_{ freq }, q_{ q }, range_low_{ range_low },
        range_high_{ range_high}, type_{ type } {
    Log(DEBUG) << "Added filter with frequency " << freq << " Q " << q
               << " range [" << range_low << ", " << range_high << "] and type "
               << type << endl;
}

void Filter::initialize(double gain) {
    double w0 = 2.0 * M_PI * freq_ / Config::get<double>(KEY_FS, 48000);
    double alpha = sin(w0) / (2 * q_);
    double beta;
    double A;
    double a0, a1, a2, b0, b1, b2;

    if (type_ == VALUE_FILTER_PEAKING) {
        A = pow(10, gain / 40);

        a0 = 1 + alpha/A;
		a1 = -2 * cos(w0);
		a2 = 1 - alpha / A;
		b0 = (1 + alpha * A);
		b1 = -(2 * cos(w0));
		b2 = (1 - alpha * A);
    } else if (type_ == VALUE_FILTER_LOW_SHELF) {
        // Invert due to algorithm
        gain = -gain;
        A = pow(10, gain / 40);
        beta = sqrt(A) / q_;

        a0 = A * ((A + 1) - (A - 1) * cos(w0) + beta * sin(w0));
		a1 = 2 * A * ((A -1) - (A + 1) * cos(w0));
		a2 = A * ((A + 1) - (A - 1) * cos(w0) - beta * sin(w0));
		b0 = (A + 1) + (A - 1) * cos(w0) + beta * sin(w0);
		b1 = -2 * ((A - 1) + (A + 1) * cos(w0));
		b2 = (A + 1) + (A - 1) * cos(w0) - beta * sin(w0);
    } else if (type_ == VALUE_FILTER_HIGH_SHELF) {
        // Invert due to algorithm
        gain = -gain;
        A = pow(10, gain / 40);
        beta = sqrt(A) / q_;

        a0 = A * ((A + 1) + (A - 1) * cos(w0) + beta * sin(w0));
		a1 = -2 * A * ((A -1) + (A + 1) * cos(w0));
		a2 = A * ((A + 1) + (A - 1) * cos(w0) - beta * sin(w0));
		b0 = (A + 1) - (A - 1) * cos(w0) + beta * sin(w0);
		b1 = 2 * ((A - 1) - (A + 1) * cos(w0));
		b2 = (A + 1) - (A - 1) * cos(w0) - beta * sin(w0);
    } else {
        Log(WARN) << "Unknown filter type " << type_ << ", ignoring\n";
        return;
    }

    a_.clear();
    b_.clear();

    a_.push_back(b0 / a0);
    b_.push_back(b1 / a0);
    b_.push_back(b2 / a0);
    b_.push_back(a1 / a0);
    b_.push_back(a2 / a0);
}

double Filter::get_gain(double freq) {
    double omega = 2 * M_PI * freq / Config::get<double>(KEY_FS, 48000);
	double sn = sin(omega / 2.0);
	double phi = sn * sn;
	double b0 = a_.front();
	double b1 = b_.front();
	double b2 = b_.at(1);
	double a0 = 1.0;
	double a1 = b_.at(2);
	double a2 = b_.at(3);

	double dbGain = 10 * log10(pow(b0 + b1 + b2, 2) - 4 * (b0 * b1 + 4 * b0 * b2 + b1 * b2) * phi + 16 * b0 * b2 * phi * phi)
		- 10 * log10(pow(a0 + a1 + a2, 2) - 4 * (a0 * a1 + 4 * a0 * a2 + a1 * a2) * phi + 16 * a0 * a2 * phi * phi);

	return dbGain;
}

void Filter::apply(Response& response, double gain) {
    initialize(gain);

    #pragma omp parallel for
    for (size_t i = 0; i < response.freqs_.size(); i++) {
        response.gains_.at(i) += get_gain(response.freqs_.at(i));
    }
}

double Filter::optimize(Response& response, const Response& target, double current_gain) {
    if (!Config::has(KEY_RESPECT_FREQ_LIMITS) || Config::get(KEY_RESPECT_FREQ_LIMITS, true)) {
        // Ignore filter if filter bandwidth is won't affect within limits
        auto low = Config::get(KEY_LOW_LIMIT, 20);
        auto high = Config::get(KEY_HIGH_LIMIT, 20000);
        if (type_ == VALUE_FILTER_HIGH_SHELF) {
            if (freq_ < low) {
                return 0;
            }
        } else if (type_ == VALUE_FILTER_LOW_SHELF) {
            if (freq_ > high) {
                return 0;
            }
        } else if (type_ == VALUE_FILTER_PEAKING) {
            if (freq_ < low || freq_ > high) {
                return 0;
            }
        }
    }

    double best = response.get_flatness(target);
    double best_gain = 0; // No filter applied
    double min_gain = range_low_ - current_gain;
    double max_gain = Config::get<double>(KEY_FILTER_MAX_GAIN, range_high_) - current_gain;

    // Go through possible gains
    // TODO: Do this better
    auto accuracy = Config::get(KEY_ACCURACY_LEVEL, 0.5);
    for (double gain = min_gain; gain <= max_gain; gain += accuracy) {
        //Log(DEBUG) << "Trying gain " << gain << " for filter with freq " << freq_ << endl;
        auto current_response = response;
        apply(current_response, gain);
        auto score = current_response.get_flatness(target);
        if (score < best) {
            best = score;
            best_gain = gain;
        }
    }

    Log(DEBUG) << "Best gain for " << freq_ << " was " << best_gain << endl;

    apply(response, best_gain);

    return best_gain;
}