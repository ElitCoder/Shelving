#pragma once

#include <vector>
#include <string>

class Filter {
public:
    explicit Filter(double freq, double q, double range_low,
                    double range_high, const std::string& type);

private:
    double freq_;
    double gain_;
    double q_;
    double range_low_;
    double range_high_;
    std::string type_;
};

using FilterBank = std::vector<Filter>;