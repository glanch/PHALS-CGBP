#include "Master.h"
#include "../Settings.h"
#include <scip/scip_cons.h>

void Master::CreateZVariable(Coil coil_i)
{
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
                 SCIP_VARTYPE_INTEGER, // variable type
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

   for (auto &coil : instance_->coils)
   {
      this->CreateZVariable(coil);
   }

   // we have currently no lambda, cause we are at the beginning of our columnGeneration-Process.

   // still, original variables need to be added

   // #####################################################################################################################
   //  Add restrictions
   // #####################################################################################################################

   // (2) coil partitioning: every regular coil is produced at exactly one line
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
   SCIPwriteOrigProblem(scipRMP_, "original_RMP_bpp.lp", "lp", FALSE);
}

/**
 * @brief Destroy the Master:: Master object
 *
 * @note This code is a destructor for a class called "Master". It is responsible for releasing all constraints,
 * variables and the SCIP (Solving Constraint Integer Programming) environment associated with the class. For each
 * generated pattern, the corresponding lambda variables (stored in the vector "_var_lambda"), SCIPreleaseVar is called
 * to release it's memory from the SCIP environment. Finally, SCIPfree is called on the SCIP environment itself, which
 * frees any memory associated with the environment. Note that constraints are not freed.
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

   SCIPfree(&scipRMP_);
}

// solve the problem
void Master::Solve()
{
   cout << "___________________________________________________________________________________________\n";
   cout << "start Solving ColumnGeneration: \n";
   SCIPsolve(scipRMP_);
}

/**
 * @brief Display the solution of the master problem
 *
 * @note The code defines a member function called "displaySolution" of a class called "Master". This function utilizes
 * the SCIP optimization solver to print the best solution found by the solver. The SCIPprintBestSol function takes
 * three arguments: a pointer to a SCIP instance (in this case, scipRMP_), a pointer to a file stream for output (in
 * this case, NULL), and a Boolean value indicating whether to display the solution in verbose mode (in this case,
 * FALSE). The function does not return any value.
 */

/**
 * @brief Set the SCIP parameters
 *
 * @note This code defines a method named setSCIPParameters in the class Master. The method sets various parameters for
 * the SCIP optimization solver. These parameters include limits for time and gap, verbosity level, and display options.
 * Additionally, a file for the vbc-tool is specified so that the branch and bound tree can be visualized.
 * Some parameters are modified specifically for the pricing process. For column generation, the maxrestarts parameter
 * is set to 0 to avoid a known bug. For constraints containing priced variables, the rootredcost parameter is disabled,
 * while constraints that may not be respected during the pricing process are not added.
 * Finally, constraints are not separated to avoid adding ones that may be violated during the pricing process.
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
tuple<bool, Coil, Mode, Mode> Master::FindSucessorCoil(SCIP_Sol *solution, Coil coil_i, ProductionLine line)
{
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
            auto var_value = SCIPgetSolVal(scipRMP_, solution, var);

            if (var_value > 0.5)
            { // TODO: use SCIP epsilon methods
               return make_tuple(true, coil_j, mode_i, mode_j);
            }
         }
      }
   }

   return make_tuple(false, 0, 0, 0);
}

void Master::DisplaySolution()
{
   
   SCIPprintBestSol(scipRMP_, NULL, FALSE);

   SCIP_SOL *solution = SCIPgetBestSol(scipRMP_);

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
};
