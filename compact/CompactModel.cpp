#include "CompactModel.h"
#include "../Settings.h"
#include <scip/scip_general.h>
#include <scip/scip_prob.h>
/**
    Creates and adds a Z variable for the specified coil.

    @param coil_i The Coil object for which to create the Z variable.
    */
void CompactModel::CreateZVariable(Coil coil_i)
{
   assert(vars_Z_.count(coil_i) == 0);
   char var_cons_name[Settings::kSCIPMaxStringLength];

   // create Z variable
   // initialize map entry, TODO: find out if this is necessary
   vars_Z_[coil_i] = nullptr;

   SCIP_VAR **z_var_pointer = &vars_Z_[coil_i];
   SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "Z_C%d", coil_i);
   SCIPcreateVarBasic(scip_,                 //
                      z_var_pointer,         // returns the address of the newly created variable
                      var_cons_name,         // name
                      0,                     // lower bound
                      1,                     // upper bound
                      0,                     // objective function coefficient, equal to 0 according to model
                      SCIP_VARTYPE_INTEGER); // variable type

   SCIPaddVar(scip_, *z_var_pointer);
}

/**
    Creates and adds a S variable for the specified coil.

    @param coil_i The Coil object for which to create the S variable.
    */
void CompactModel::CreateSVariable(Coil coil_i)
{
   assert(vars_S_.count(coil_i) == 0);
   // bounds of variable: if this is start coil, variable should be equal to 0, else 0 <= var <= +infty
   SCIP_Real lb = instance_->IsStartCoil(coil_i) ? 0 : 0;
   SCIP_Real ub = instance_->IsStartCoil(coil_i) ? 0 : SCIPinfinity(scip_);

   char var_cons_name[Settings::kSCIPMaxStringLength];

   // initialize variable ptr in map
   vars_S_[coil_i] = nullptr;

   SCIP_VAR **s_var_pointer = &vars_S_[coil_i];
   SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "S_C%d", coil_i);
   SCIPcreateVarBasic(scip_,                    //
                      s_var_pointer,            // returns the address of the newly created variable
                      var_cons_name,            // name
                      lb,                       // lower bound, see above
                      ub,                       // upper bound, see above
                      0,                        // objective function coefficient, equal to 0 according to model
                      SCIP_VARTYPE_CONTINUOUS); // variable type

   SCIPaddVar(scip_, *s_var_pointer);
}

void CompactModel::CreateXVariable(Coil coil_i, Coil coil_j, ProductionLine line, Mode mode_i, Mode mode_j)
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
   SCIPcreateVarBasic(scip_,                                                                      //
                      x_var_pointer,                                                              // returns the address of the newly created variable
                      var_cons_name,                                                              // name
                      lb,                                                                         // lower bound
                      ub,                                                                         // upper bound
                      instance_->stringerCosts[make_tuple(coil_i, mode_i, coil_j, mode_j, line)], // objective function coefficient, this is equal to c_ijkmn. If c_ijkmn is not presented, c_ijkmn is initialized with 0 and 0 is returned, i.e. coefficient is 0
                      SCIP_VARTYPE_INTEGER);                                                      // variable type

   SCIPaddVar(scip_, *x_var_pointer);
}

/**
 * @brief Construct a new Compact Model:: Compact Model object
 *
 * @param instance pointer to problem-instance
 *
 * @note This code is a constructor for the CompactModel class. It creates a SCIP environment and sets the specific
 * parameters. It then creates and adds all variables to the model, including binary variables X_ijkmn, Z_i and Z_i for items i,
 * bins j. Finally, it adds all restrictions to the model and writes the final LP-model into a file.
 */
CompactModel::CompactModel(shared_ptr<Instance> instance) : instance_(instance)
{
   // create a SCIP environment and load all defaults
   SCIPcreate(&scip_);
   SCIPincludeDefaultPlugins(scip_);

   // create an empty model
   SCIPcreateProbBasic(scip_, "Compact Model PHALS");

   // set the objective sense to minimize (not mandatory, default is minimize)
   SCIPsetObjsense(scip_, SCIP_OBJSENSE_MINIMIZE);

   // call the created function set all optional SCIPParameters
   this->SetSCIPParameters();

   // create helping-dummy for the name of variables and constraints
   char var_cons_name[Settings::kSCIPMaxStringLength];

   // #####################################################################################################################
   //  Create and add all variables
   // #####################################################################################################################

   // constant variable
   SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "Const");
   SCIPcreateVarBasic(scip_,                    //
                      &var_constant_one_,       // returns the address of the newly created variable
                      var_cons_name,            // name
                      1,                        // lower bound
                      1,                        // upper bound
                      0,                        // objective function coefficient, equal to 0 according to model
                      SCIP_VARTYPE_CONTINUOUS); // variable type

   SCIPaddVar(scip_, var_constant_one_);

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
            // }
         }
      }
   }

   for (auto &coil : instance_->coils)
   {
      this->CreateZVariable(coil);
      this->CreateSVariable(coil);
   }

   // #####################################################################################################################
   //  Add restrictions
   // #####################################################################################################################

   // (2) coil partitioning: every regular coil is produced at exactly one line
   for (auto &coil_i : instance_->regularCoils)
   {
      SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "coil_partitioning_%d", coil_i);

      SCIPcreateConsBasicLinear(scip_,                            // scip
                                &cons_coil_partitioning_[coil_i], // cons
                                var_cons_name,                    // name
                                0,                                // nvar
                                0,                                // vars
                                0,                                // coeffs
                                1,                                // lhs
                                1);                               // rhs

      // add coefficients
      for (auto &line : instance_->productionLines)
      {
         // skip start coil since it may occur at multiple lines
         for (auto &coil_j : instance_->coilsWithoutStartCoil)
         {
            for (auto &mode_i : instance_->modes[make_tuple(coil_i, line)])
            {
               for (auto &mode_j : instance_->modes[make_tuple(coil_j, line)])
               {
                  auto tuple = make_tuple(coil_i, coil_j, line, mode_i, mode_j);
                  auto &var_X = vars_X_[make_tuple(coil_i, coil_j, line, mode_i, mode_j)];
                  SCIPaddCoefLinear(scip_, cons_coil_partitioning_[coil_i], var_X, 1);
               }
            }
         }
      }

      SCIPaddCons(scip_, cons_coil_partitioning_[coil_i]);
   }

   // (3) production line start: every line has exactly one successor of starting coil
   for (auto &line : instance_->productionLines)
   {
      SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "production_line_start_%d", line);

      SCIPcreateConsBasicLinear(scip_,                              // scip
                                &cons_production_line_start_[line], // cons
                                var_cons_name,                      // name
                                0,                                  // nvar
                                0,                                  // vars
                                0,                                  // coeffs
                                1,                                  // lhs
                                1);                                 // rhs

      // add coefficients
      Coil coil_i = instance_->startCoil; // this is the start coil
      
      // skip sentinel coils, only regular coils
      for (auto &coil_j : instance_->regularCoils)
      {  
         for (auto &mode_i : instance_->modes[make_tuple(coil_i, line)])
         {
            for (auto &mode_j : instance_->modes[make_tuple(coil_j, line)])
            {
               SCIPaddCoefLinear(scip_, cons_production_line_start_[line], vars_X_[make_tuple(coil_i, coil_j, line, mode_i, mode_j)], 1);
            }
         }
      }
      SCIPaddCons(scip_, cons_production_line_start_[line]);
   }

   // (4) production line end: every line has exactly one predecessor of end coil
   for (auto &line : instance_->productionLines)
   {
      SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "production_line_end_%d", line);

      SCIPcreateConsBasicLinear(scip_,                            // scip
                                &cons_production_line_end_[line], // cons
                                var_cons_name,                    // name
                                0,                                // nvar
                                0,                                // vars
                                0,                                // coeffs
                                1,                                // lhs
                                1);                               // rhs

      // add coefficients
      Coil coil_j = instance_->endCoil; // this is the end coil
      // skip sentinel coils, only regular coils
      for (auto &coil_i : instance_->regularCoils)
      {
         for (auto &mode_i : instance_->modes[make_tuple(coil_i, line)])
         {
            for (auto &mode_j : instance_->modes[make_tuple(coil_j, line)])
            {
               auto tuple = make_tuple(coil_i, coil_j, line, mode_i, mode_j);
               auto &var_X = vars_X_[tuple];
               SCIPaddCoefLinear(scip_, cons_production_line_end_[line], var_X, 1);
            }
         }
      }
      SCIPaddCons(scip_, cons_production_line_end_[line]);
   }

   // (5) flow conservation: for every line, every coil, and every corresponding mode there is equal amount of incoming and outgoing edges
   for (auto &line : instance_->productionLines)
   {
      // skip sentinel coils, only regular coils
      for (auto &coil_j : instance_->regularCoils)
      {

         auto coil_j_line_tuple = make_tuple(coil_j, line);
         for (auto &mode_j : instance_->modes[coil_j_line_tuple])
         {
            auto cons_tuple = make_tuple(line, coil_j, mode_j);
            SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "cons_flow_conservation_L%d_C%d_M%d", line, coil_j, mode_j);

            SCIPcreateConsBasicLinear(scip_,                                // scip
                                      &cons_flow_conservation_[cons_tuple], // cons
                                      var_cons_name,                        // name
                                      0,                                    // nvar
                                      0,                                    // vars
                                      0,                                    // coeffs
                                      0,                                    // lhs
                                      0);                                   // rhs

            // add coefficients

            // positive part of LHS: incoming edges
            // skip end coil and include start coil
            for (auto &coil_i : instance_->coilsWithoutEndCoil)
            {
               for (auto &mode_i : instance_->modes[make_tuple(coil_i, line)])
               {
                  SCIPaddCoefLinear(scip_, cons_flow_conservation_[cons_tuple], vars_X_[make_tuple(coil_i, coil_j, line, mode_i, mode_j)], 1);
               }
            }

            // negative part of LHS: outgoing edges
            // skip start coil and include end coil
            for (auto &coil_i : instance_->coilsWithoutStartCoil)
            {
               if (instance_->IsStartCoil(coil_i))
                  continue;

               for (auto &mode_i : instance_->modes[make_tuple(coil_i, line)])
               {
                  SCIPaddCoefLinear(scip_, cons_flow_conservation_[cons_tuple], vars_X_[make_tuple(coil_j, coil_i, line, mode_j, mode_i)], -1);
               }
            }

            SCIPaddCons(scip_, cons_flow_conservation_[cons_tuple]);
         }
      }
   }

   // (6) delay linking: link Z_i, S_i and X_..
   // skip non-regular coils
   for (auto &coil_i : instance_->regularCoils)
   {
      SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "delay_linking_C%d", coil_i);

      SCIPcreateConsBasicLinear(scip_,                        // scip
                                &cons_delay_linking_[coil_i], // cons
                                var_cons_name,                // name
                                0,                            // nvar
                                0,                            // vars
                                0,                            // coeffs
                                -SCIPinfinity(scip_),         // lhs
                                0);                           // rhs

      // add coefficients
      // add S_i
      SCIPaddCoefLinear(scip_, cons_delay_linking_[coil_i], vars_S_[coil_i], 1);

      // other
      for (auto &line : instance_->productionLines)
      {
         // if coil_j is start coil, skip
         for (auto &coil_j : instance_->coilsWithoutStartCoil)
         {
            for (auto &mode_j : instance_->modes[make_tuple(coil_j, line)])
            {
               for (auto &mode_i : instance_->modes[make_tuple(coil_i, line)])
               {
                  auto processing_time = instance_->processingTimes[make_tuple(coil_i, line, mode_i)];

                  SCIPaddCoefLinear(scip_, cons_delay_linking_[coil_i], vars_X_[make_tuple(coil_i, coil_j, line, mode_i, mode_j)], processing_time);
               }
            }
         }
      }

      // RHS
      // due date d_i
      SCIPaddCoefLinear(scip_, cons_delay_linking_[coil_i], var_constant_one_, -instance_->dueDates[coil_i]);

      // big M linearization
      SCIP_Real big_M = Settings::kBigM; // TODO: !!!!
      SCIPaddCoefLinear(scip_, cons_delay_linking_[coil_i], vars_Z_[coil_i], -big_M);

      SCIPaddCons(scip_, cons_delay_linking_[coil_i]);
   }

   // (7) start time linking: link S_i and X_..
   // skip non-regular coils for both coil_i and coil_j
   for (auto &coil_i : instance_->regularCoils)
   {
      for (auto &coil_j : instance_->regularCoils)
      {

         auto con_tuple = make_tuple(coil_i, coil_j);

         SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "start_time_linking_CI%d_CJ%d", coil_i, coil_j);

         SCIPcreateConsBasicLinear(scip_,                                // scip
                                   &cons_start_time_linking_[con_tuple], // cons
                                   var_cons_name,                        // name
                                   0,                                    // nvar
                                   0,                                    // vars
                                   0,                                    // coeffs
                                   -SCIPinfinity(scip_),                 // lhs
                                   0);                                   // rhs

         // add coefficients
         // add S_i
         SCIPaddCoefLinear(scip_, cons_start_time_linking_[con_tuple], vars_S_[coil_i], 1);

         // add -S_j
         SCIPaddCoefLinear(scip_, cons_start_time_linking_[con_tuple], vars_S_[coil_j], -1);

         // add -M
         SCIP_Real big_M = Settings::kBigM;
         SCIPaddCoefLinear(scip_, cons_start_time_linking_[con_tuple], var_constant_one_, -big_M);

         for (auto &line : instance_->productionLines)
         {
            for (auto &mode_i : instance_->modes[make_tuple(coil_i, line)])
            {
               for (auto &mode_j : instance_->modes[make_tuple(coil_j, line)])
               {
                  // (p_ikm+tijkmn)*X_ijkmn
                  auto processing_time = instance_->processingTimes[make_tuple(coil_i, line, mode_i)];
                  auto setup_time = instance_->setupTimes[make_tuple(coil_i, mode_i, coil_j, mode_j, line)];
                  SCIP_Real coefficient = processing_time + setup_time;

                  SCIPaddCoefLinear(scip_, cons_start_time_linking_[con_tuple], vars_X_[make_tuple(coil_i, coil_j, line, mode_i, mode_j)], coefficient);

                  // M*X_ijkmn
                  SCIPaddCoefLinear(scip_, cons_start_time_linking_[con_tuple], vars_X_[make_tuple(coil_i, coil_j, line, mode_i, mode_j)], big_M);
               }
            }
         }
         SCIPaddCons(scip_, cons_start_time_linking_[con_tuple]);
      }
   }

   // (8) max number of delayed columns
   SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "max_delayed_coils");

   SCIPcreateConsBasicLinear(scip_,                           // scip
                             &cons_max_delayed_coils_,        // cons
                             var_cons_name,                   // name
                             0,                               // nvar
                             0,                               // vars
                             0,                               // coeffs
                             -SCIPinfinity(scip_),            // lhs
                             instance_->maximumDelayedCoils); // rhs
   // skip non-regular coils
   for (auto &coil_i : instance_->regularCoils)
   {
      SCIPaddCoefLinear(scip_, cons_max_delayed_coils_, vars_Z_[coil_i], 1);
   }

   SCIPaddCons(scip_, cons_max_delayed_coils_);

   // Generate a file to show the LP-Program, that is build. "FALSE" = we get our specific choosen names.
   SCIPwriteOrigProblem(scip_, "compact_model_PHALS.lp", "lp", FALSE);
}

/**
 * @brief Destroy the Compact Model:: Compact Model object
 *
 * @note This is the destructor for the Compact Model class. It releases all constraints and variables associated with
 * the model, and then releases the SCIP object. It releases all unique-item-assignment constraints and all
 * capacity-and-linking constraints. It also releases all
 * X_ij -variables and Y_i -variables Finally it frees the SCIP object.
 * If you get a:
// "WARNING: Original variable <> not released when freeing SCIP problem <>"
// this is the place to look and check every constraint and variable (yes, also the constraints, it leads to the same
// warning).
 */
CompactModel::~CompactModel()
{
   for (auto &[_, cons] : this->cons_coil_partitioning_)
   {
      SCIPreleaseCons(scip_, &cons);
   }

   for (auto &[_, cons] : this->cons_production_line_start_)
   {
      SCIPreleaseCons(scip_, &cons);
   }

   for (auto &[_, cons] : this->cons_production_line_end_)
   {
      SCIPreleaseCons(scip_, &cons);
   }

   for (auto &[_, cons] : this->cons_flow_conservation_)
   {
      SCIPreleaseCons(scip_, &cons);
   }

   for (auto &[_, cons] : this->cons_delay_linking_)
   {
      SCIPreleaseCons(scip_, &cons);
   }

   for (auto &[_, cons] : this->cons_start_time_linking_)
   {
      SCIPreleaseCons(scip_, &cons);
   }

   SCIPreleaseCons(scip_, &cons_max_delayed_coils_);

   for (auto &[_, var] : this->vars_X_)
   {
      SCIPreleaseVar(scip_, &var);
   }

   for (auto &[_, var] : this->vars_S_)
   {
      SCIPreleaseVar(scip_, &var);
   }

   for (auto &[_, var] : this->vars_Z_)
   {
      SCIPreleaseVar(scip_, &var);
   }

   SCIPreleaseVar(scip_, &var_constant_one_);

   SCIPfree(&scip_);
}

/**
 * @brief set optional SCIP parameters
 *
 * @note This function sets optional SCIP parameters for the CompactModel object. The parameters that are set are
 * "limits/time" to 1e+20 seconds, "limits/gap" to 0, "display/verblevel" to 4, and "display/lpinfo" to FALSE. For more
 * information on these parameters, please refer to the SCIP documentation at
 * https://www.scipopt.org/doc/html/PARAMETERS.php.
 */
void CompactModel::SetSCIPParameters()
{
   SCIPsetRealParam(scip_, "limits/time", 1e+20);    // default 1e+20 s
   SCIPsetRealParam(scip_, "limits/gap", 0);         // default 0
   SCIPsetIntParam(scip_, "display/verblevel", 4);   // default 4
   SCIPsetBoolParam(scip_, "display/lpinfo", FALSE); // default FALSE
};

/**
 * @brief solve the compact model
 *
 * @note This function solves the compact model using SCIPsolve. It prints a message to the console indicating that it
 * is starting to solve the compact model.
 */
void CompactModel::Solve()
{
   cout << "___________________________________________________________________________________________\n";
   cout << "start Solving compact Model: \n";
   SCIPsolve(scip_);
};

/**
 * @brief Display every Value of the variables in the optimal solution
 *
 * @note This function displays every value of the variables in the optimal solution of a CompactModel object. It takes
 * no parameters and returns nothing. It uses the SCIPprintBestSol() function from the SCIP library to print out the
 * values.
 */

tuple<bool, Coil, Mode, Mode> CompactModel::FindSucessorCoil(SCIP_Sol *solution, Coil coil_i, ProductionLine line)
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
            auto var_value = SCIPgetSolVal(scip_, solution, var);

            if (var_value > 0.5)
            { // TODO: use SCIP epsilon methods
               return make_tuple(true, coil_j, mode_i, mode_j);
            }
         }
      }
   }

   return make_tuple(false, 0, 0, 0);
}
void CompactModel::DisplaySolution()
{
   SCIPprintBestSol(scip_, NULL, FALSE);

   SCIP_SOL *solution = SCIPgetBestSol(scip_);

   cout << endl
        << endl;
   cout << "==== Coil Assignment ====" << endl;
   for (auto &line : instance_->productionLines)
   {
      cout << "Line " << line << endl;
      Coil coil_i = instance_->startCoil;
      auto [found, coil_j, mode_i, mode_j] = FindSucessorCoil(solution, coil_i, line);

      assert(found);

      while (coil_i != instance_->endCoil)
      {
         if (coil_i == instance_->startCoil)
         {
            cout << "Start";
         }
         else
         {
            cout << "C" << coil_i << "M" << mode_i;
            cout << " t=" << SCIPgetSolVal(scip_, solution, vars_S_[coil_i]);
            if (SCIPgetSolVal(scip_, solution, vars_Z_[coil_i]) > 0.5)
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

      cout << "End" << endl;

      cout << "==== Coil Assignment ====" << endl;
   }
};
