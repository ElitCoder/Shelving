#include "Log.h"
#include "Config.h"
#include "Calculate.h"

#include <vector>

using namespace std;

bool parse() {
    // Parse common config
    // TODO: Make path dynamic
    if (!Config::parse_common("config/common.conf")) {
        Log(ERR) << "Failed to parse common config\n";
        return false;
    }

    return true;
}

bool run() {
    // Make sure input file and input type is set
    if (!Config::has(KEY_INPUT) || !Config::has(KEY_INPUT_TYPE)) {
        Log(ERR) << "No input file or input type specified in config\n";
        return false;
    }

    return calculate();
}

int main() {
    if (!parse()) {
        return false;
    }

    // Parse commands and execute
    if (!run()) {
        Log(ERR) << "Fatal error, exiting\n";
        return -1;
    }

    return 0;
}