#include "Log.h"
#include "Config.h"

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
    return true;
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