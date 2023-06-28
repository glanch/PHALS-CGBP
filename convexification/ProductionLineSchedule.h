struct ProductionLineSchedule {
    SCIP_Real reduced_cost;
    SCIP_Real schedule_cost;
    ProductionLine line;
    map<tuple<Coil, Coil, ProductionLine, Mode, Mode>, bool> edges;
    int lambda_pattern_index;
};