#pragma once

#include "Filter.h"

class Response;

class Optimize {
public:
    static void optimize(const Response& response, const Response& target);
};