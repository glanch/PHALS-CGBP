#include "Master.h"
#include "../Settings.h"
#include <scip/scip_cons.h>
#include <numeric>
/**
 * @brief Create a original binary Z_i variable and create/add it to the original variable constraint
 * 
 * @param coil_i The coil i that the Z_i variable is created for
 */
void Master::CreateZVariable(Coil coil_i)
{
   // sanity check: don't create variable more than once
   assert(vars_Z_.count(coil_i) == 0);

   char var_cons_name[Settings::kSCIPMaxStringLength];

   // create Z variable
   // initialize map entry, TODO: find out if this is necessary
   vars_Z_[coil_i] = nullptr;

   SCIP_VAR **z_var_pointer = &vars_Z_[coil_i];
   SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "Z_C%d", coil_i);
   SCIPcreateVar(scipRMP_,             //
                 z_var_pointer,        // returns the address of the newly created variable
                 var_cons_name,        // name
                 0,                    // lower bound
                 1,                    // upper bound
                 0,                    // objective function coefficient, equal to 0 according to model
                 SCIP_VARTYPE_IMPLINT, // variable type
                 true,
                 false,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 NULL);

   SCIPaddVar(scipRMP_, *z_var_pointer);

   // (C) original variable constraints
   // add constraints to orig var constraint
   // Z_i = 0
   // since set of lambdas is empty, no other terms in constraint 
   SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "orig_var_Z_C%d", coil_i);

   SCIPcreateConsLinear(scipRMP_,                     // scip
                        &cons_original_var_Z[coil_i], // cons
                        var_cons_name,                // name
                        0,                            // nvar
                        0,                            // vars
                        0,                            // coeffs
                        0,                            // lhs
                        0,                            // rhs
                        TRUE,                         // initial
                        FALSE,                        // separate
                        TRUE,                         // enforce
                        TRUE,                         // check
                        TRUE,                         // propagate
                        FALSE,                        // local
                        TRUE,                         // modifiable
                        FALSE,                        // dynamic
                        FALSE,                        // removable
                        FALSE);                       // stick at nodes

   SCIPaddCoefLinear(scipRMP_, cons_original_var_Z[coil_i], *z_var_pointer, 1);
   SCIPaddCons(scipRMP_, cons_original_var_Z[coil_i]);
}

/**
 * @brief Create a original binary S_i variable and create/add it to the original variable constraint
 * 
 * @param coil_i The coil i that the S_i variable is created for
 */
void Master::CreateSVariable(Coil coil_i)
{
   assert(vars_S_.count(coil_i) == 0);
   // bounds of variable: if this is start coil, variable should be equal to 0, else 0 <= var <= +infty
   SCIP_Real lb = instance_->IsStartCoil(coil_i) ? 0 : 0;
   SCIP_Real ub = instance_->IsStartCoil(coil_i) ? 0 : SCIPinfinity(scipRMP_);

   char var_cons_name[Settings::kSCIPMaxStringLength];

   // initialize variable ptr in map
   vars_S_[coil_i] = nullptr;

   SCIP_VAR **s_var_pointer = &vars_S_[coil_i];
   SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "S_C%d", coil_i);
   SCIPcreateVar(scipRMP_,                //
                 s_var_pointer,           // returns the address of the newly created variable
                 var_cons_name,           // name
                 lb,                      // lower bound, see above
                 ub,                      // upper bound, see above
                 0,                       // objective function coefficient, equal to 0 according to model
                 SCIP_VARTYPE_CONTINUOUS, // variable type
                 true,
                 false,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 NULL);

   SCIPaddVar(scipRMP_, *s_var_pointer);

   // (D) original variable constraints
   // add constraints to orig var constraint
   // S_i = 0
   // since set of lambdas is empty, no other terms in constraint 

   SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "orig_var_S_C%d", coil_i);

   SCIPcreateConsLinear(scipRMP_,                     // scip
                        &cons_original_var_S[coil_i], // cons
                        var_cons_name,                // name
                        0,                            // nvar
                        0,                            // vars
                        0,                            // coeffs
                        0,                            // lhs
                        0,                            // rhs
                        TRUE,                         // initial
                        FALSE,                        // separate
                        TRUE,                         // enforce
                        TRUE,                         // check
                        TRUE,                         // propagate
                        FALSE,                        // local
                        TRUE,                         // modifiable
                        FALSE,                        // dynamic
                        FALSE,                        // removable
                        FALSE);                       // stick at nodes

   SCIPaddCoefLinear(scipRMP_, cons_original_var_S[coil_i], *s_var_pointer, 1);
   SCIPaddCons(scipRMP_, cons_original_var_S[coil_i]);
}

/**
 * @brief Create a original binary X_ijkmn variable and create/add it to the original variable constraint
 * 
 * @param coil_i Coil i this variable is created for
 * @param coil_j Coil j this variable is created for
 * @param line Production line this variable is created for
 * @param mode_i Mode m of coil i this variable is created for
 * @param mode_j Mode n of coil j this variable is created for
 */
void Master::CreateXVariable(Coil coil_i, Coil coil_j, ProductionLine line, Mode mode_i, Mode mode_j)
{
   // bounds of variable: if coil_i = coil_j, variable should be equal to 0, else binary, i.e. 0 <= var <= 0
   SCIP_Real lb = (coil_i == coil_j) ? 0 : 0;
   SCIP_Real ub = (coil_i == coil_j) ? 0 : 1;

   char var_cons_name[Settings::kSCIPMaxStringLength];

   // initialize variable ptr in map
   auto var_tuple = make_tuple(coil_i, coil_j, line, mode_i, mode_j);
   assert(vars_X_.count(var_tuple) == 0);
   SCIP_VAR **x_var_pointer = &vars_X_[var_tuple];
   SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "X_CI%d_CJ%d_L%d_MI%d_MJ%d", coil_i, coil_j, line, mode_i, mode_j);
   SCIPcreateVar(scipRMP_,             //
                 x_var_pointer,        // returns the address of the newly created variable
                 var_cons_name,        // name
                 lb,                   // lower bound
                 ub,                   // upper bound
                 0,                    // objective function coefficient, this is equal to zero since only lambda variables occur
                 SCIP_VARTYPE_INTEGER, // variable type
                 true,
                 false,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 NULL);

   SCIPaddVar(scipRMP_, *x_var_pointer);

   // (B) original variable constraints
   // add constraints to orig var constraint
   SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "orig_var_X_CI%d_CJ%d_L%d_MI%d_MJ%d", coil_i, coil_j, line, mode_i, mode_j);

   SCIPcreateConsLinear(scipRMP_,                        // scip
                        &cons_original_var_X[var_tuple], // cons
                        var_cons_name,                   // name
                        0,                               // nvar
                        0,                               // vars
                        0,                               // coeffs
                        0,                               // lhs
                        0,                               // rhs
                        TRUE,                            // initial
                        FALSE,                           // separate
                        TRUE,                            // enforce
                        TRUE,                            // check
                        TRUE,                            // propagate
                        FALSE,                           // local
                        TRUE,                            // modifiable
                        FALSE,                           // dynamic
                        FALSE,                           // removable
                        FALSE);                          // stick at nodes

   SCIPaddCoefLinear(scipRMP_, cons_original_var_X[var_tuple], *x_var_pointer, 1);
   SCIPaddCons(scipRMP_, cons_original_var_X[var_tuple]);
}

/**
 * @brief Construct a new Master:: Master object. In this the SCIP object
 * containing the MIP is created and variables and constraints added.
 * 
 * @param instance A reference to the instance the Master problem is created for
 */
Master::Master(shared_ptr<Instance> instance) : instance_(instance), initial_column_heuristic_tried_(false)
{
   // create a SCIP environment and load all defaults
   SCIPcreate(&scipRMP_);
   SCIPincludeDefaultPlugins(scipRMP_);

   // create an empty problem
   SCIPcreateProb(scipRMP_, "master-problem PHALS", 0, 0, 0, 0, 0, 0, 0);

   // set the objective sense to minimize (not mandatory, default is minimize)
   SCIPsetObjsense(scipRMP_, SCIP_OBJSENSE_MINIMIZE);

   // set all optional SCIPParameters
   this->SetSCIPParameters();

   // create Helping-dummy for the name of variables and constraints
   char var_cons_name[Settings::kSCIPMaxStringLength];

   // #####################################################################################################################
   //  Create and add all variables
   // #####################################################################################################################

   // constant variable
   SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "Const");
   SCIPcreateVar(scipRMP_,           //
                 &var_constant_one_, // returns the address of the newly created variable
                 var_cons_name,      // name
                 1,                  // lower bound
                 1,                  // upper bound
                 0,                  // objective function coefficient, equal to 0 according to model
                 SCIP_VARTYPE_CONTINUOUS,
                 true,
                 false,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 NULL);

   SCIPaddVar(scipRMP_, var_constant_one_);

   // create X_ijkmn variables
   for (auto &line : instance_->productionLines)
   {
      for (auto &coil_i : instance_->coilsWithoutEndCoil)
      {
         for (auto &coil_j : instance_->coilsWithoutStartCoil)
         {
            // if coil_i and coil_j are both sentinel coils, there should be no connection, thus skip this iteration,else, continue
            if (instance_->IsStartCoil(coil_i) && instance_->IsEndCoil(coil_j))
            {
               continue;
            }
            // if (coil_i != coil_j)
            // {
            // TODO: check this!
            auto &modes_i = instance_->modes[make_tuple(coil_i, line)];
            auto &modes_j = instance_->modes[make_tuple(coil_j, line)];

            for (auto &mode_i : modes_i)
            {
               for (auto &mode_j : modes_j)
               {
                  CreateXVariable(coil_i, coil_j, line, mode_i, mode_j);
               }
            }
         }
      }
   }


   // Create Z_i variable
   for (auto &coil : instance_->coils)
   {
      this->CreateZVariable(coil);
   }

   // we have currently no lambda, cause we are at the beginning of our column generation process.  
   
   // #####################################################################################################################
   //  Add restrictions
   // #####################################################################################################################

   // (2) coil partitioning: every regular coil is produced at exactly one line
   // for every regular coil i introduce constraint
   // sum(coil incl. end coil j, line k, mode m of coil i on line k, mode n of coil j on line k) X_ijkmn = 1 
   // equivalent to
   // 1<= sum(coil incl. end coil j, line k, mode m of coil i on line k, mode n of coil j on line k) X_ijkmn <= 1
   for (auto &coil_i : instance_->regularCoils)
   {
      SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "coil_partitioning_%d", coil_i);

      SCIPcreateConsLinear(scipRMP_,                         // scip
                           &cons_coil_partitioning_[coil_i], // cons
                           var_cons_name,                    // name
                           0,                                // nvar
                           0,                                // vars
                           0,                                // coeffs
                           1,                                // lhs
                           1,                                // rhs
                           TRUE,                             // initial
                           FALSE,                            // separate
                           TRUE,                             // enforce
                           TRUE,                             // check
                           TRUE,                             // propagate
                           FALSE,                            // local
                           TRUE,                             // modifiable
                           FALSE,                            // dynamic
                           FALSE,                            // removable
                           FALSE);                           // stick at nodes

      // no coefficients here since no columns generated yet

      SCIPaddCons(scipRMP_, cons_coil_partitioning_[coil_i]);
   }

   // (8) max number of delayed columns
   // -infty sum(regular coil, Z_i) <= alpha
   SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "max_delayed_coils");

   SCIPcreateConsLinear(scipRMP_,                       // scip
                        &cons_max_delayed_coils_,       // cons
                        var_cons_name,                  // name
                        0,                              // nvar
                        0,                              // vars
                        0,                              // coeffs
                        -SCIPinfinity(scipRMP_),        // lhs
                        instance_->maximumDelayedCoils, // rhs
                        TRUE,                           // initial
                        FALSE,                          // separate
                        TRUE,                           // enforce
                        TRUE,                           // check
                        TRUE,                           // propagate
                        FALSE,                          // local
                        TRUE,                           // modifiable
                        FALSE,                          // dynamic
                        FALSE,                          // removable
                        FALSE);                         // stick at nodes

   // no coefficients here since no columns generated yet

   SCIPaddCons(scipRMP_, cons_max_delayed_coils_);

   // (A) convexity constraints
   // for every production line, sum of lambda variables = 1
   // i.e. 1<= sum(production line k, lambdas, 1*lambda) <= 1
   // currently 0 = 1 since no lambda variable are available yet

   for (auto &line : instance_->productionLines)
   {
      SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "convexity_L%d", line);

      SCIPcreateConsLinear(scipRMP_,               // scip
                           &cons_convexity_[line], // cons
                           var_cons_name,          // name
                           0,                      // nvar
                           0,                      // vars
                           0,                      // coeffs
                           1,                      // lhs
                           1,                      // rhs
                           TRUE,                   // initial
                           FALSE,                  // separate
                           TRUE,                   // enforce
                           TRUE,                   // check
                           TRUE,                   // propagate
                           FALSE,                  // local
                           TRUE,                   // modifiable
                           FALSE,                  // dynamic
                           FALSE,                  // removable
                           FALSE);                 // stick at nodes

      // no columns yet, so no vars to be added

      SCIPaddCons(scipRMP_, cons_convexity_[line]);
   }

   // generate a file to show the LP-Program that is build. "FALSE" = we get our specific choosen names.
   SCIPwriteOrigProblem(scipRMP_, "original_RMP_PHALS.lp", "lp", FALSE);

   // create timer for measuring
   SCIPcreateClock(scipRMP_, &master_round_clock);
}

/**
 * @brief Destroy the Master:: Master object. This destroy the SCIP objects
 * (object itself, variables, clock and constraints). Every other ressource is
 * freed automatically if necessary
 */
Master::~Master()
{

   for (auto &[_, cons] : this->cons_convexity_)
   {
      SCIPreleaseCons(scipRMP_, &cons);
   }

   for (auto &[_, cons] : this->cons_coil_partitioning_)
   {
      SCIPreleaseCons(scipRMP_, &cons);
   }

   for (auto &[_, cons] : this->cons_original_var_X)
   {
      SCIPreleaseCons(scipRMP_, &cons);
   }

   for (auto &[_, cons] : this->cons_original_var_Z)
   {
      SCIPreleaseCons(scipRMP_, &cons);
   }

   for (auto &[_, cons] : this->cons_original_var_S)
   {
      SCIPreleaseCons(scipRMP_, &cons);
   }

   SCIPreleaseCons(scipRMP_, &cons_max_delayed_coils_);

   // free variables
   for (auto &[_, var] : this->vars_X_)
   {
      SCIPreleaseVar(scipRMP_, &var);
   }

   for (auto &[_, var] : this->vars_S_)
   {
      SCIPreleaseVar(scipRMP_, &var);
   }

   for (auto &[_, var] : this->vars_Z_)
   {
      SCIPreleaseVar(scipRMP_, &var);
   }

   for (auto &[_, vars] : this->vars_lambda_)
   {
      for (auto &var : vars)
      {
         SCIPreleaseVar(scipRMP_, &var);
      }
   }

   SCIPreleaseVar(scipRMP_, &var_constant_one_);

   // free clock
   SCIPfreeClock(scipRMP_, &master_round_clock);

   // free instance
   SCIPfree(&scipRMP_);
}

// solve the problem
/**
 * @brief Solve the Master problem using Branch&Price algorithm
 * 
 * @param time_limit The time limit after the solution process should terminate
 */
void Master::Solve(double time_limit)
{
   cout << "___________________________________________________________________________________________" << endl;
   cout << "start Solving ColumnGeneration:" << endl;
   cout << "Time Limit: " << time_limit << endl;

   // set time limit
   SCIPsetRealParam(scipRMP_, "limits/time", time_limit);

   // start timer by calling restart
   RestartTimer();

   // trigger B&P algorithm
   SCIPsolve(scipRMP_);

   // measure time after solution
   auto last_measure = MeasureTime("Master Last Measure");


   // print all measured timings
   double total_time = 0.0;

   cout << "Master Timings" << endl;
   for (auto &[description, measured_time] : master_round_timings_in_seconds)
   {
      cout << description << ": " << measured_time << "s" << endl;
      total_time += measured_time;
   }

   // get scips total solving time
   auto total_solving_time = SCIPgetSolvingTime(scipRMP_);

   // print measured times
   cout << "=== TOTAL TIME IN LP SOLVING === " << std::fixed << total_time << endl;
   cout << "=== TOTAL TIME IN B&P === " << std::fixed << total_solving_time << endl;
}

/**
 * @brief Set SCIP of the SCIP object. This includes default time limit, default
 * gap, verb level and lp info. This also enables visualization of B&P tree in
 * VBC format. Furthermore, separation is disabled
 */
void Master::SetSCIPParameters()
{
   // for more information: https://www.scipopt.org/doc/html/PARAMETERS.php
   SCIPsetRealParam(scipRMP_, "limits/time", 1e+20);    // default 1e+20 s
   SCIPsetRealParam(scipRMP_, "limits/gap", 0);         // default 0
   SCIPsetIntParam(scipRMP_, "display/verblevel", 4);   // default 4
   SCIPsetBoolParam(scipRMP_, "display/lpinfo", FALSE); // default FALSE

   // write a file for vbc-tool, so that later the branch&bound tree can be visualized
   SCIPsetStringParam(scipRMP_, "visual/vbcfilename", "tree.vbc");

   // modify some parameters so that the pricing can work properly

   // http://scip.zib.de : "known bug : If one uses column generation and restarts, a solution that contains
   // variables that are only present in the transformed problem
   //(i.e., variables that were generated by a pricer) is not pulled back into the original space correctly,
   // since the priced variables have no original counterpart.Therefore, one should disable restarts by setting the
   // parameter "presolving/maxrestarts" to 0, if one uses a column generation approach."

   SCIPsetIntParam(scipRMP_, "presolving/maxrestarts", 0);

   // http://scip.zib.de : "If your pricer cannot cope with variable bounds other than 0 and infinity, you have to
   // mark all constraints containing priced variables as modifiable, and you may have to disable reduced cost
   // strengthening by setting propagating / rootredcost / freq to - 1."
   SCIPsetIntParam(scipRMP_, "propagating/rootredcost/freq", -1);

   // no separation to avoid that constraints are added which we cannot respect during the pricing process
   SCIPsetSeparating(scipRMP_, SCIP_PARAMSETTING_OFF, TRUE);
}

/**
 * @brief Given an optimal SCIP solution, finds the successor coil of coil i on
 * line k
 *
 * @param solution SCIP solution on which the successor coil should be searched
 * for
 * @param coil_i 
 * @param line 
 * @return tuple<bool, Coil, Mode, Mode> First entry of tuple specifies if
 * sucessor coil was found. If yes, the second entry specifies successor coil j
 * of coil i. The third entry then specifies the mode of coil i. The fourth
 * entry specifies the mode of successor coil j.
 */
tuple<bool, Coil, Mode, Mode> Master::FindSucessorCoil(SCIP_Sol *solution, Coil coil_i, ProductionLine line)
{
   // check every possible X_ijkmn for given i and k (line)
   for (auto &mode_i : instance_->modes[make_tuple(coil_i, line)])
   {
      for (auto &coil_j : instance_->coils)
      {
         for (auto &mode_j : instance_->modes[make_tuple(coil_j, line)])
         {
            auto var_tuple = make_tuple(coil_i, coil_j, line, mode_i, mode_j);
            // skip variables that don't exist
            if (vars_X_.count(var_tuple) == 0)
               continue;

            auto &var = vars_X_[var_tuple];

            // get variable value
            auto var_value = SCIPgetSolVal(scipRMP_, solution, var);
            // if variable value is sufficiently large, here at least 1/2, variable value of binry variable is interpreted as true
            if (var_value > 0.5)
            { // TODO: use SCIP epsilon methods
               return make_tuple(true, coil_j, mode_i, mode_j);
            }
         }
      }
   }

   return make_tuple(false, 0, 0, 0);
}

/**
 * @brief Displays the yet best found solution and statistics of solving
 * process. If reconstruction of production schedules is enabled, the method
 * FindSucessorCoil(..) is iteratively used to reconstruct production schedule
 * for every production line
 *
 */
void Master::DisplaySolution()
{
   cout << "=== STATS ===" << endl;
   SCIPprintPricerStatistics(scipRMP_, NULL);
   SCIPprintBestSol(scipRMP_, NULL, FALSE);
   cout << "Time: " << SCIPgetDeterministicTime(scipRMP_);

   SCIP_SOL *solution = SCIPgetBestSol(scipRMP_);

   if (Settings::kReconstructScheduleFromSolution)
   {

      cout << endl
           << endl;

      cout << "==== Coil Assignment ====" << endl;

      double total_cost = 0;
      for (auto &line : instance_->productionLines)
      {
         // calculate total cost and find successor coils, starting from coil_i=0, mode=0
         double line_cost = 0;
         cout << "Line " << line << endl;
         Coil coil_i = instance_->startCoil;
         auto [found, coil_j, mode_i, mode_j] = FindSucessorCoil(solution, coil_i, line);

         assert(found);

         while (coil_i != instance_->endCoil)
         {
            line_cost += instance_->stringerCosts[make_tuple(coil_i, mode_i, coil_j, mode_j, line)];
            if (coil_i == instance_->startCoil)
            {
               cout << "Start";
            }
            else
            {
               cout << "C" << coil_i << "M" << mode_i;
               // cout << " t=" << SCIPgetSolVal(scipRMP_, solution, vars_S_[coil_i]);
               if (SCIPgetSolVal(scipRMP_, solution, vars_Z_[coil_i]) > 0.5)
                  cout << " delayed";
            }
            cout << " -> ";
            coil_i = coil_j;

            auto [found_new, coil_j_new, mode_i_new, mode_j_new] = FindSucessorCoil(solution, coil_i, line);
            found = found_new;
            coil_j = coil_j_new;
            mode_i = mode_i_new;
            mode_j = mode_j_new;
         }

         // update total cost
         total_cost += line_cost;

         cout << "End" << endl;
         cout << "Line cost: " << line_cost << endl;
         cout << "==== Coil Assignment ====" << endl;
      }
      cout << "Total cost: " << total_cost << endl;
   }
};

/**
 * @brief Stop the SCIP clock and write a log entry of measured time
 * 
 * @param description The description of the log entry
 * @return SCIP_Real The measured time
 */
SCIP_Real Master::MeasureTime(string description)
{
   SCIPstopClock(scipRMP_, master_round_clock);
   auto measured_time = SCIPgetClockTime(scipRMP_, master_round_clock);

   master_round_timings_in_seconds.push_back(make_tuple(description, measured_time));

   return measured_time;
}
/**
 * @brief Restart the SCIP clock
 * 
 */
void Master::RestartTimer()
{
   SCIPresetClock(scipRMP_, master_round_clock);
   SCIPstartClock(scipRMP_, master_round_clock);
}