#include "Optimize.h"
#include "Config.h"

// Iterate through all filters one at a time and loop
void Optimize::optimize(const Response& response, const Response& target) {
    auto bank = Config::get_filter_bank();

    for (int i = 0; i < 1 /* ITERATIONS */; i++) {
        for (auto& filter : bank) {
            auto current_response = response;
            auto gain = filter.optimize(current_response, target);
        }

        // See current filter status
    }
}