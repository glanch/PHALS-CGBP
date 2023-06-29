#pragma once
#include <scip/scip_general.h>
#include <scip/scip_prob.h>
#include "../Settings.h"
#include "../Instance.h"
#include "ProductionLineSchedule.h"
#include "DualValues.h"
class SubProblem
{
public:
    SubProblem();
    ~SubProblem();
    void Setup(shared_ptr<Instance> instance, ProductionLine line);

    void UpdateObjective(shared_ptr<DualValues> dual_values, const bool is_farkas);
    shared_ptr<ProductionLineSchedule> Solve();
private:
    ProductionLine line_;
    shared_ptr<Instance> instance_;
    SCIP *scipSP_;

    // variables
    // dummy variable
    SCIP_VAR *var_constant_one_;
    map<tuple<Coil, Coil, ProductionLine, Mode, Mode>, SCIP_VAR *> vars_X_;
    map<Coil, SCIP_VAR *> vars_Z_;
    map<Coil, SCIP_VAR *> vars_S_;
    
    SCIP_CONS *cons_production_line_start_;
    SCIP_CONS *cons_production_line_end_;
    map<tuple<Coil, Mode>, SCIP_CONS *> cons_flow_conservation_;
    map<Coil, SCIP_CONS *> cons_delay_linking_;
    map<tuple<Coil, Coil>, SCIP_CONS *> cons_start_time_linking_;
};