#include "Filter.h"
#include "Log.h"

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
}

double Filter::get_gain(double freq) {
    return 0;
}

void Filter::apply(Response& response, double gain) {
    initialize(gain);

    for (size_t i = 0; i < response.freqs_.size(); i++) {
        response.gains_.at(i) += get_gain(response.freqs_.at(i));
    }
}

double Filter::optimize(const Response& response, const Response& target) {
    // Calculate +- x dB before filter
    // TODO
    auto best = response.get_flatness(target);
    auto best_gain = 0; // No filter applied

    // Go through possible gains
    // TODO: Do this better
    for (double gain = range_low_; gain <= range_high_; gain += 1) {
        Log(DEBUG) << "Trying gain " << gain << " for filter with freq " << freq_ << endl;
        auto current_response = response;
        apply(current_response, gain);
        auto score = current_response.get_flatness(target);
        if (score < best) {
            best = score;
            best_gain = gain;
        }
    }

    return best_gain;
}