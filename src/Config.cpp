#include "Config.h"
#include "Log.h"

#include <fstream>

using namespace std;

// [freq] [q] [range low] [range high] [type]
const int FILTER_VALUE_TOKENS = 5;

map<string, string> Config::config_;
FilterBank Config::filter_bank_;

static vector<string> split(string input, const string& delimiter) {
    size_t pos;
    string token;
    vector<string> splitted;
    while ((pos = input.find(delimiter)) != string::npos) {
        token = input.substr(0, pos);
        splitted.push_back(token);
        input.erase(0, pos + delimiter.length());
    }
    // Add last token
    splitted.push_back(input);

    return splitted;
}

bool Config::add_key(const string& key, const string& value) {
    if (key == "eq") {
        // Parse EQ
        if (!parse_common(value)) {
            Log(ERR) << "Failed to parse EQ " << value << endl;
            return false;
        }
    } else if (key == "filter") {
        // Add filter
        // Split on whitespace
        auto splitted = split(value, " ");
        if (splitted.size() != FILTER_VALUE_TOKENS) {
            Log(WARN) << "Specified filter " << value << " is malformed\n";
            return true; // Not an error
        }
        auto freq = stod(splitted[0]);
        auto q = stod(splitted[1]);
        auto range_low = stod(splitted[2]);
        auto range_high = stod(splitted[3]);
        auto type = splitted[4];
        filter_bank_.push_back(Filter(freq, q, range_low, range_high, type));
    } else {
        // Just add the key
        auto found = config_.find(key);
        if (found != config_.end()) {
            Log(WARN) << "Key " << key << " already exists with value "
                      << found->second << ", replacing it with value "
                      << value << endl;
        }

        config_[key] = value;
    }

    return true;
}

bool Config::parse_common(const string& filename) {
    ifstream file(filename);
    if (!file) {
        Log(ERR) << "Failed to open config " << filename << endl;
        return false;
    }

    // Parse line by line
    string line;
    while (!file.eof()) {
        getline(file, line);
        // Ignore empty lines or comments, '#'
        if (line.empty() || line.find_first_of('#') != string::npos) {
            continue;
        }

        // Split on " = "
        auto splitted = split(line, " = ");
        // Should be splitted into [key, value]
        if (splitted.size() != 2) {
            Log(WARN) << "Line " << line << " is malformed, ignoring\n";
            continue;
        }

        auto& key = splitted.front();
        auto& value = splitted.back();
        Log(DEBUG) << "Found key " << key << " with value " << value << endl;
        add_key(key, value);
    }

    file.close();
    return true;
}