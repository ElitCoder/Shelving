#include "Response.h"
#include "Log.h"
#include "Filter.h"
#include "Calculate.h"

#include <fstream>
#include <algorithm>
#include <numeric>
#include <cmath>

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

    Log(DEBUG) << "Starting input parsing..\n";

    // Parse in REW format 'freq gain phase'
    vector<double> freqs;
    vector<double> gains;
    while (!file.eof()) {
        string cmt;
        file >> cmt;
        if (cmt.empty() || cmt.front() == '*') {
            // Comment
            getline(file, cmt);
            continue;
        }

        double freq, gain, phase;
        freq = stod(cmt);
        file >> gain;
        file >> phase;
        freqs.push_back(freq);
        gains.push_back(gain);
        // Ignore phase

#if 0
        if (lround(freq) % 500 == 0) {
            Log(DEBUG) << "Parsed freq " << freq << " with gain " << gain << " to be at index " << freqs.size() << endl;
        }
#endif
    }

    Log(DEBUG) << "Input parsing done!\n";

    file.close();
    return Response(freqs, gains);
}

double Response::get_flatness(const Response& target) const {
    vector<double> diff;

    for (size_t i = 0; i < target.freqs_.size(); i++) {
        if (target.freqs_.at(i) < 60 || target.freqs_.at(i) > 20000) {
            continue;
        }

        diff.push_back(abs(target.gains_.at(i) - gains_.at(i)));
    }

    double sum = accumulate(diff.begin(), diff.end(), 0) / (double)diff.size();

    return sum;
}

void Response::print() const {
    Log(NONE) << "\nResponse\n";
    for (size_t i = 0; i < freqs_.size(); i++) {
        Log(NONE) << freqs_.at(i) << " " << gains_.at(i) << endl;
    }
    Log(NONE) << "GAINS COPY FRIENDLY ";
    for (auto& gain : gains_) {
        Log(NONE) << gain << " ";
    }
    Log(NONE) << endl << endl;
}