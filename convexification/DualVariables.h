
class DualVariables
{
public:
    DualVariables(shared_ptr<Instance> instance) : instance_(instance) {}

private:
    shared_ptr<Instance> instance_;

    map<Coil, SCIP_REAL> pi_partitioning;
    map<ProductionLine, SCIP_REAL> pi_convexity;
    SCIP_REAL pi_max_delayed_coils;
    map<tuple<Coil, Coil, Mode, ProductionLine, Mode, Mode>, SCIP_REAL> pi_max_delayed_coils;
}
