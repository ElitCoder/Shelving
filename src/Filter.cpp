#include "Filter.h"
#include "Log.h"

using namespace std;

Filter::Filter(double freq, double q, double range_low, double range_high,
               const string& type) :
        freq_{ freq }, q_{ q }, range_low_{ range_low },
        range_high_{ range_high}, type_{ type } {
    Log(DEBUG) << "Added filter with frequency " << freq << " Q " << q
               << " range [" << range_low << ", " << range_high << "] and type "
               << type << endl;
}