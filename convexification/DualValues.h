#pragma once
#include<memory>
/**
 * @brief Container for dual solutions
 *
 * This struct acts as a container for dual solutions. For every constraint in a
 * Master problem instance, a SCIP_Real containing the corresponding dual value
 * is populated. This struct can be used for both dual solutions and Farkas
 * multipliers.
 */
class DualValues
{
public:
    DualValues(shared_ptr<Instance> instance) : instance_(instance) {}
    shared_ptr<Instance> instance_;

    //pi from Master
    map<Coil, SCIP_Real> pi_partitioning_;
    map<ProductionLine, SCIP_Real> pi_convexity_;
    SCIP_Real pi_max_delayed_coils_;
    map<tuple<Coil, Coil, ProductionLine, Mode, Mode>, SCIP_Real> pi_original_var_X;
    map<Coil, SCIP_Real> pi_original_var_Z;

};
