#pragma once
#include<memory>
class DualValues
{
public:
    DualValues(shared_ptr<Instance> instance) : instance_(instance) {}
    shared_ptr<Instance> instance_;

    map<Coil, SCIP_Real> pi_partitioning_;
    map<ProductionLine, SCIP_Real> pi_convexity_;
    SCIP_Real pi_max_delayed_coils_;
    map<tuple<Coil, Coil, ProductionLine, Mode, Mode>, SCIP_Real> pi_original_var_X;
    map<Coil, SCIP_Real> pi_original_var_Z;

};
