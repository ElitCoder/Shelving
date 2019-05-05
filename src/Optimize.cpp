#include "Optimize.h"
#include "Config.h"

// Iterate through all filters one at a time and loop
void Optimize::optimize(const Response& response, const Response& target) {
    auto bank = Config::get_filter_bank();

    for (int i = 0; i < 10 /* ITERATIONS */; i++) {
        auto current_response = response;

        for (auto& filter : bank) {
            auto gain = filter.optimize(current_response, target);
        }

        // See current filter status
    }
}