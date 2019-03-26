#include "Response.h"

using namespace std;

Response::Response(const vector<double>& freqs, const vector<double>& gains) :
        freqs_{ freqs }, gains_{ gains } {}