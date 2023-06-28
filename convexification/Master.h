// Master.h
#pragma once

#include <memory>

#include "../Instance.h"

// scip includes
#include "objscip/objbenders.h"
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

#include "ProductionLineSchedule.h"
using namespace scip;

/**
 * @brief a class to store all informations for the master problem to solve the BPP with column generation
 *
 * @param _scipRMP pointer to the scip environment for the restricted master-problem
 * @param _ins pointer to the instance
 * @param _var_lambda  decision-variables
 * @param _cons various SCIP constraints
 *
 */
class Master
{
public:
   Master(shared_ptr<Instance> instance); // constructor

   ~Master(); // destructor

   SCIP *scipRMP_;      // pointer to the scip environment for the restricted master-problem
   shared_ptr<Instance> instance_; // pointer to the instance

   // Variables
   map<ProductionLine, vector<SCIP_VAR *>> vars_lambda_;
   map<tuple<Coil, Coil, ProductionLine, Mode, Mode>, SCIP_VAR *> vars_X_;
   map<Coil, SCIP_VAR *> vars_Z_;
   map<Coil, SCIP_VAR *> vars_S_;

   // dummy variable
   SCIP_VAR *var_constant_one_;

   // column coefficients
   map<ProductionLine, vector<ProductionLineSchedule>> schedules_;

   map<Coil, SCIP_CONS *> cons_convexity_;
   map<Coil, SCIP_CONS *> cons_coil_partitioning_;
   SCIP_CONS *cons_max_delayed_coils_;
   
   map<tuple<Coil, Coil, ProductionLine, Mode, Mode>, SCIP_CONS *> cons_original_var_X;
   map<Coil, SCIP_CONS *> cons_original_var_Z;
   map<Coil, SCIP_CONS *> cons_original_var_S;
   
   // solve the problem void solve();
   void Solve();

   // display the solution
   void DisplaySolution();

   // set all optional SCIP-Parameters
   void SetSCIPParameters();

   void CreateZVariable(Coil coil_i);
   void CreateSVariable(Coil coil_i);
   void CreateXVariable(Coil coil_i, Coil coil_j, ProductionLine line, Mode mode_i, Mode mode_j);

   tuple<bool, Coil, Mode, Mode> FindSucessorCoil(SCIP_Sol *solution, Coil coil_i, ProductionLine line);
};
