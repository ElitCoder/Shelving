#pragma once

#include "Filter.h"

#include <map>
#include <vector>

class Config {
public:
    static bool parse_common(const std::string& filename);

private:
    static bool add_key(const std::string& key, const std::string& value);

    static std::map<std::string, std::string> config_;
    static FilterBank filter_bank_;
};