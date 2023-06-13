// CompactModel.h
#pragma once

#include "Instance.h"

/* scip includes */
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"
#include <memory>
#include <map>
#include "Settings.h"
using namespace scip;

class CompactModel
{

public:
   // constructor
   CompactModel(shared_ptr<Instance> instance);

   // destructor
   ~CompactModel();

   // solve the problem
   void Solve();

   // display the solution
   void DisplaySolution();

   // set all optional SCIP-Parameters
   void SetSCIPParameters();

private:
   constexpr char[] var_Z_fmt = "Z_%d";
   shared_ptr<Instance> instance_;
   SCIP* scip_;

   // sentinels: start and end coils
   Coil start_coil_;
   Coil end_coil_;

   // variables
   map<tuple<Coil, Coil, ProductionLine, Mode, Mode>, SCIP_VAR*> vars_X_;

   map<Coil, SCIP_VAR*> vars_Z_;

   map<Coil, SCIP_VAR*> vars_S_;
   
   // constraints
   map<Coil, SCIP_CONS*> cons_coil_partitioning_;
   map<ProductionLine, SCIP_CONS*> cons_production_line_start_;

   map<ProductionLine, SCIP_CONS*> cons_production_line_end_;

   map<tuple<ProductionLine, Coil, Mode>, SCIP_CONS*> cons_flow_conservation_;

   map<Coil, SCIP_CONS*> cons_delay_linking_;

   map<pair<Coil, Coil>, SCIP_CONS*> cons_start_time_linking_;
};
