#include "Log.h"
#include "Config.h"

#include <vector>

using namespace std;

void print_help(const string& name) {
    Log(NONE) << "Usage: " << name << " [OPTIONS]\n";
}

bool parse(int argc, char** argv) {
    // Parse common config
    // TODO: Make path dynamic
    if (!Config::parse_common("config/common.conf")) {
        Log(ERR) << "Failed to parse common config\n";
        return false;
    }

    // Parse CMD
    vector<string> args(argv, argv + argc - 1);
    if (!Config::parse_cmd(argc, args)) {
        print_help(argv[0]);
        return false;
    }

    return true;
}

bool run() {
    return true;
}

int main(int argc, char** argv) {
    if (!parse(argc, argv)) {
        return false;
    }

    // Parse commands and execute
    if (!run()) {
        Log(ERR) << "Fatal error, exiting\n";
        return -1;
    }

    return 0;
}