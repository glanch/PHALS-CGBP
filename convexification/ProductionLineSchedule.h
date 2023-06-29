#pragma once
#include <scip/scip_general.h>

struct ProductionLineSchedule {
    SCIP_Real reduced_cost;
    bool reduced_cost_negative;
    SCIP_Real schedule_cost;
    ProductionLine line;
    map<tuple<Coil, Coil, ProductionLine, Mode, Mode>, bool> edges;
    map<Coil, bool> delayedness;
    int lambda_index;
};