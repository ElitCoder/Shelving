#pragma once

#include "Filter.h"

#include <map>
#include <vector>
#include <sstream>

// Keys, perhaps move them somewhere else
const std::string KEY_INPUT = "input";
const std::string KEY_INPUT_TYPE = "input_type";
const std::string KEY_FS = "fs";
const std::string KEY_LOW_LIMIT = "low_limit";
const std::string KEY_HIGH_LIMIT = "high_limit";

// Values
const std::string VALUE_INPUT_TYPE_FR = "frequency_response";
const std::string VALUE_INPUT_TYPE_RAW_PINK = "raw_pink";
const std::string VALUE_INPUT_TYPE_RAW_WHITE = "raw_white";
// Filter values
const std::string VALUE_FILTER_PEAKING = "peaking";

class Config {
public:
    static bool parse_common(const std::string& filename);
    static bool has(const std::string& key);
    static void set(const std::string& key, const std::string& value);

    template<class T>
    static T get(const std::string& key, const T& default_value) {
        if (!has(key)) {
            return default_value;
        }

        std::istringstream stream(config_[key]);
        T value;
        stream >> value;

        return value;
    }

    static FilterBank get_filter_bank();

private:
    static bool add_key(const std::string& key, const std::string& value);

    static std::map<std::string, std::string> config_;
    static FilterBank filter_bank_;
};