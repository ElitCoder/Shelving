#include "Optimize.h"
#include "Config.h"
#include "Log.h"

using namespace std;

// Iterate through all filters one at a time and loop
void Optimize::optimize(const Response& response, const Response& target) {
    auto bank = Config::get_filter_bank();

    vector<double> best_gains(bank.size(), 0);
    auto current_response = response;
    for (int i = 0; i < 100 /* ITERATIONS */; i++) {
        vector<double> iteration_gains(bank.size(), 0);

        for (size_t j = 0; j < bank.size(); j++) {
            auto& filter = bank.at(j);
            auto gain = filter.optimize(current_response, target, best_gains.at(j));
#if 0
            Log(DEBUG) << "For filter " << filter.freq_ << endl;
            Log(DEBUG) << "BEFORE " << response.get_flatness(target) << "\n";
            response.print();
            Log(DEBUG) << "AFTER " << current_response.get_flatness(target) << "\n";
            current_response.print();
            Log(DEBUG) << "With TARGET " << target.get_flatness(target) << "\n";
            target.print();
#endif
            best_gains.at(j) += gain;
            iteration_gains.at(j) += gain;
        }

        bool done = true;
        for (auto& gain : iteration_gains) {
            if (gain != 0) {
                done = false;
            }
        }

        if (done) {
            // Nothing changed, we're done
            break;
        }
    }

    Log(DEBUG) << "Best gains: ";
    for (auto& gain : best_gains) {
        Log(NONE) << gain << " ";
    }
    Log(NONE) << endl;
}