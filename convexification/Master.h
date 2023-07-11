// Master.h
#pragma once

#include <memory>
#include <mutex>

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

/**
 * @brief A class to store all information for the Master problem to solve the PHALS model with column generation
 */
class Master
{
public:
   Master(shared_ptr<Instance> instance); // constructor

   ~Master(); // destructor

   // Mutex for synchronisation of concurrent access
   std::mutex mutex_;

   // pointer to the scip environment for the restricted master-problem
   SCIP *scipRMP_;      

   // pointer to the instance
   shared_ptr<Instance> instance_; 

   // List of schedules per production line, i.e. column coefficients
   map<ProductionLine, vector<shared_ptr<ProductionLineSchedule>>> schedules_; // TODO: refactor into one combined map with lambda vars
   
   // Variables
   // Lambda variables per production line
   map<ProductionLine, vector<SCIP_VAR *>> vars_lambda_;

   // original variables X_ijkmn
   map<tuple<Coil, Coil, ProductionLine, Mode, Mode>, SCIP_VAR *> vars_X_;
   // original variables Z_i
   map<Coil, SCIP_VAR *> vars_Z_;
   // original variables S_i
   map<Coil, SCIP_VAR *> vars_S_;

   // dummy variable that is always equal to one in every feasible solution
   SCIP_VAR *var_constant_one_;

   // convexity constraint for every production line
   map<Coil, SCIP_CONS *> cons_convexity_;

   // coil partitioning constraint for every coil: every coil is scheduled exactly once
   map<Coil, SCIP_CONS *> cons_coil_partitioning_;

   // maximum delayed coils constraint
   SCIP_CONS *cons_max_delayed_coils_;
   
   // constraints for restoring original variables from lambda values
   // cons for variable X_ijkmn
   map<tuple<Coil, Coil, ProductionLine, Mode, Mode>, SCIP_CONS *> cons_original_var_X;
   
   // cons for variable Z_i
   map<Coil, SCIP_CONS *> cons_original_var_Z;

   // cons for variable S_i
   map<Coil, SCIP_CONS *> cons_original_var_S;
   
   // solve the problem void solve();
   void Solve(double time_limit);

   // display the solution
   void DisplaySolution();

   // set all optional SCIP-Parameters
   void SetSCIPParameters();

   // Create variables
   void CreateZVariable(Coil coil_i);
   void CreateSVariable(Coil coil_i);
   void CreateXVariable(Coil coil_i, Coil coil_j, ProductionLine line, Mode mode_i, Mode mode_j);

   // Find successor coil
   tuple<bool, Coil, Mode, Mode> FindSucessorCoil(SCIP_Sol *solution, Coil coil_i, ProductionLine line);

   // Experiment: try to find most covering production plan, disabled
   bool initial_column_heuristic_tried_ = false;
   bool initial_column_heuristic_enabled_ = false;

   // methods for measuring tim
   SCIP_Real MeasureTime(string description);
   void RestartTimer();


   // pointer to scip clock
   SCIP_CLOCK* master_round_clock;
   
   // list of measurements of master_round_clock
   vector<tuple<string, double>> master_round_timings_in_seconds;
};
