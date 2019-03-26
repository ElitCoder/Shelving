#include "Response.h"
#include "Log.h"
#include "Filter.h"

#include <fstream>

using namespace std;

Response::Response() {}

Response::Response(const vector<double>& freqs, const vector<double>& gains) :
        freqs_{ freqs }, gains_{ gains } {}

// TODO: Move this
Response Response::parse(const string& filename) {
    ifstream file(filename);
    if (!file) {
        Log(WARN) << "Failed to parse response " << filename << endl;
        return Response();
    }

    // Parse in REW format 'freq gain phase'
    vector<double> freqs;
    vector<double> gains;
    while (!file.eof()) {
        double freq, gain, phase;
        file >> freq;
        file >> gain;
        file >> phase;
        freqs.push_back(freq);
        gains.push_back(gain);
        // Ignore phase
    }

    file.close();
    return Response(freqs, gains);
}

double Response::get_flatness(const Response& target) const {
    // Response-objects should have the exact same frequency vectors
    if (freqs_.size() != target.freqs_.size()) {
        Log(WARN) << "Frequency vectors are not of same size\n";
        return -10000000;
    }

    // TODO
    return 0;
}

void Response::apply(const Filter& filter) {
    for (size_t i = 0; i < freqs_.size(); i++) {
        gains_.at(i) += filter.get_gain(freqs_.at(i));
    }

    // TODO
}