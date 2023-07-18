#pragma once
#include <scip/scip_general.h>
// Struct capturing a production line schedule for a given line
struct ProductionLineSchedule {
    SCIP_Real reduced_cost = 0;
    bool reduced_cost_negative = false;
    SCIP_Real schedule_cost = 0;
    ProductionLine line = 0;
    map<tuple<Coil, Coil, ProductionLine, Mode, Mode>, bool> edges;
    map<Coil, bool> delayedness;
    int lambda_index = 0;
};