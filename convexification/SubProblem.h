#pragma once

#include "../Settings.h"
#include "../Instance.h"

class SubProblem
{
public:
    SubProblem(shared_ptr<Instance> instance, ProductionLine line);

private:
    ProductionLine line_;

    SCIP_CONS *cons_production_line_start_;
    SCIP_CONS *cons_production_line_end_;
    map<tuple<Coil, Mode>, SCIP_CONS *> cons_flow_conservation_;
    map<Coil, SCIP_CONS *> cons_delay_linking_;
    map<tuple<Coil, Coil>, SCIP_CONS *> cons_start_time_linking_;
}