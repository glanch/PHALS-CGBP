// CompactModel.h
#pragma once

#include "../Instance.h"
#include "../Settings.h"

/* scip includes */
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"
#include <memory>
#include <map>
using namespace scip;

class CompactModel
{

public:
   // constructor
   CompactModel(shared_ptr<Instance> instance);

   // destructor
   ~CompactModel();

   // solve the problem
   void Solve(double time_limit);

   // display the solution
   void DisplaySolution();

   // set all optional SCIP-Parameters
   void SetSCIPParameters();

private:
   shared_ptr<Instance> instance_;
   SCIP *scip_;

   // variables
   // dummy variable
   SCIP_VAR *var_constant_one_;
   map<tuple<Coil, Coil, ProductionLine, Mode, Mode>, SCIP_VAR *> vars_X_;

   map<Coil, SCIP_VAR *> vars_Z_;

   map<Coil, SCIP_VAR *> vars_S_;

   // constraints
   map<Coil, SCIP_CONS *> cons_coil_partitioning_;
   map<ProductionLine, SCIP_CONS *> cons_production_line_start_;

   map<ProductionLine, SCIP_CONS *> cons_production_line_end_;

   map<tuple<ProductionLine, Coil, Mode>, SCIP_CONS *> cons_flow_conservation_;

   map<Coil, SCIP_CONS *> cons_delay_linking_;

   map<tuple<Coil, Coil>, SCIP_CONS *> cons_start_time_linking_;

   SCIP_CONS *cons_max_delayed_coils_;

   void CreateZVariable(Coil coil_i);
   void CreateSVariable(Coil coil_i);
   void CreateXVariable(Coil coil_i, Coil coil_j, ProductionLine line, Mode mode_i, Mode mode_j);
   tuple<bool, Coil, Mode, Mode> FindSucessorCoil(SCIP_Sol *solution, Coil coil_i, ProductionLine line);
};
