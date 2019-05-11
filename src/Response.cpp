#include "Response.h"
#include "Log.h"
#include "Filter.h"
#include "Calculate.h"

#include <fstream>
#include <algorithm>
#include <numeric>

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
    }

    file.close();
    return Response(freqs, gains);
}

double Response::get_flatness(const Response& target) const {
    double sum = 0;
    for (size_t i = 0; i < target.freqs_.size(); i++) {
        if (target.freqs_.at(i) < 60 || target.freqs_.at(i) > 15000)
            continue;

        sum += abs(target.gains_.at(i) - gains_.at(i));
    }

    return sum;
}