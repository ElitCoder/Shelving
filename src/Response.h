#pragma once

#include <vector>

class Response {
public:
    explicit Response(const std::vector<double>& freqs,
                      const std::vector<double>& gains);

private:
    std::vector<double> freqs_;
    std::vector<double> gains_;
};