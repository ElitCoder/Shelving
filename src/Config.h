#pragma once

#include "Filter.h"

#include <map>
#include <vector>

// [freq] [q] [range low] [range high] [type]
const int FILTER_VALUE_TOKENS = 5;

class Config {
public:
    static bool parse_cmd(int argc, const std::vector<std::string>& args);
    static bool parse_common(const std::string& filename);

private:
    static bool add_key(const std::string& key, const std::string& value);

    static std::map<std::string, std::string> config_;
    static FilterBank filter_bank_;
};