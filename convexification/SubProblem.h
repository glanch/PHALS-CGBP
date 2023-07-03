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
    vector<shared_ptr<ProductionLineSchedule>> Solve();
    
    void SetGap(double gap);
    void ResetDynamicGap();
    double dynamic_gap_ = Settings::kDynamicGap;
    
    ProductionLine line_;
private:
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
    map<Coil, SCIP_CONS *> cons_delay_edge_linking_;
    map<tuple<Coil, Coil>, SCIP_CONS *> cons_start_time_linking_;

    SCIP_CONS* cons_max_delayed_coils_;
    
    int iteration_ = 0;
    void CreateZVariable(Coil coil_i);
    void CreateSVariable(Coil coil_i);
    void CreateXVariable(Coil coil_i, Coil coil_j, ProductionLine line, Mode mode_i, Mode mode_j);

    int gap_ = 0;
};