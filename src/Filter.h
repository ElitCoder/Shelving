#pragma once

#include "Response.h"

#include <vector>
#include <string>

class Filter {
public:
    explicit Filter(double freq, double q, double range_low,
                    double range_high, const std::string& type);

    double optimize(const Response& response, const Response& target);

private:
    void initialize(double gain);
    void apply(Response& response, double gain);
    double get_gain(double freq);

    // Parametric parameters
    double freq_;
    double q_;
    double range_low_;
    double range_high_;
    std::string type_;

    // Coeffs
    double b0, b1, b2, a1, a2;
};

using FilterBank = std::vector<Filter>;