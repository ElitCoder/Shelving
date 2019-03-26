#include "Calculate.h"
#include "Config.h"
#include "Log.h"

using namespace std;

bool calculate() {
    // By this point the config should be populated and we can just calculate
    // everything we need
    auto input_type = Config::get<string>(KEY_INPUT_TYPE, "");
    if (input_type == VALUE_INPUT_TYPE_FR) {
        // Calculate optimal filter settings for specified FR
    } else if (input_type == VALUE_INPUT_TYPE_RAW_PINK) {
        Log(ERR) << "Not implemented\n";
        return false;
    } else if (input_type == VALUE_INPUT_TYPE_RAW_WHITE) {
        Log(ERR) << "Not implemented\n";
        return false;
    }

    return true;
}