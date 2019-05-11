#include "Optimize.h"
#include "Config.h"
#include "Log.h"

using namespace std;

// Iterate through all filters one at a time and loop
void Optimize::optimize(const Response& response, const Response& target) {
    auto bank = Config::get_filter_bank();

    vector<double> best_gains(bank.size(), 0);
    auto current_response = response;
    for (int i = 0; i < 5 /* ITERATIONS */; i++) {
        for (int j = 0; j < bank.size(); j++) {
            auto& filter = bank.at(j);
            auto gain = filter.optimize(current_response, target);
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
        }

        // See current filter status
    }

    Log(DEBUG) << "Best gains: ";
    for (auto& gain : best_gains) {
        Log(NONE) << gain << " ";
    }
    Log(NONE) << endl;
}