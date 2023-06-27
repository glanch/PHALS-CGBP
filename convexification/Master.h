// Master.h
#pragma once

#include "../Instance.h"

// scip includes
#include "objscip/objbenders.h"
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

#include "Pattern.h"
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

   SCIP *scipRMP_; // pointer to the scip environment for the restricted master-problem
   Instance *_ins; // pointer to the instance

   // Variables
   map<ProductionLine, vector<SCIP_VAR*> var_lambda_;
   map<tuple<Coil, Coil, ProductionLine, Mode, Mode>, SCIP_VAR *> vars_X_;
   map<Coil, vector<SCIP_VAR*> var_original_Z_;

   vector<ProductionLineSchedule> schedules_;

   map<Coil, SCIP_CONS *> cons_convexity_;
   map<Coil, SCIP_CONS *> cons_coil_partitioning_;
   SCIP_CONS *cons_max_delayed_coils_;
   map<tuple<Coil, Coil, Mode, ProductionLine,Mode, Mode>, SCIP_CONS *> cons_original_variables_X;
   map<Coil, SCIP_CONS *> cons_original_variables_Z;

   // solve the problem void solve();
   void Solve();

   // display the solution
   void DisplaySolution();

   // set all optional SCIP-Parameters
   void SetSCIPParameters();
};
