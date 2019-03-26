#pragma once

#include <vector>
#include <string>

class Filter;

class Response {
public:
    explicit Response();
    explicit Response(const std::vector<double>& freqs,
                      const std::vector<double>& gains);

    static Response parse(const std::string& filename);
    double get_flatness(const Response& target) const;
    void apply(const Filter& filter);

private:
    std::vector<double> freqs_;
    std::vector<double> gains_;
};