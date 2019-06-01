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
const std::string KEY_TARGET_SPL = "target_spl";
const std::string KEY_ACCURACY_LEVEL = "accuracy_level";
const std::string KEY_DATA_POINTS = "data_points";
const std::string KEY_HC_SLOPE_ENABLE = "slope_enable";
const std::string KEY_HC_LOUDNESS_ENABLE = "loudness_enable";
const std::string KEY_HC_SHELF_ENABLE = "shelf_enable";
const std::string KEY_HC_SLOPE_PER_OCTAVE = "slope_db_per_octave";
const std::string KEY_HC_LOUDNESS_MONITOR = "loudness_monitor";
const std::string KEY_HC_LOUDNESS_PLAYBACK = "loudness_playback";
const std::string KEY_HC_SHELF_BASS_FREQ = "shelf_bass_freq";
const std::string KEY_HC_SHELF_BASS_GAIN = "shelf_bass_gain";
const std::string KEY_HC_SHELF_TREBLE_FREQ = "shelf_treble_freq";
const std::string KEY_HC_SHELF_TREBLE_GAIN = "shelf_treble_gain";
const std::string KEY_TARGET_LOW_LIMIT = "target_low_limit";
const std::string KEY_TARGET_HIGH_LIMIT = "target_high_limit";
const std::string KEY_FILTER_MAX_GAIN = "filter_max_gain";

// Values
const std::string VALUE_INPUT_TYPE_FR = "frequency_response";
const std::string VALUE_INPUT_TYPE_RAW_PINK = "raw_pink";
const std::string VALUE_INPUT_TYPE_RAW_WHITE = "raw_white";
// Filter values
const std::string VALUE_FILTER_PEAKING = "peaking";
const std::string VALUE_FILTER_LOW_SHELF = "low_shelf";
const std::string VALUE_FILTER_HIGH_SHELF = "high_shelf";

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