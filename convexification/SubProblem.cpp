#include "SubProblem.h"
#include <memory>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
/**
 * @brief Sets the gap of the subproblem
 * 
 * @param gap The gap to be set
 */
void SubProblem::SetGap(double gap)
{
  gap_ = gap;
  SCIPsetRealParam(scipSP_, "limits/gap", gap_); // default 0
}

/**
 * @brief Sets time limit of the solution process
 * 
 * @param time_limit The time limit to be set
 */
void SubProblem::SetTimeLimit(double time_limit)
{
  SCIPsetRealParam(scipSP_, "limits/time", time_limit); // default 0
}

/**
 * @brief Resets time limit of the solution process to default value of 1+e20
 * 
 */
void SubProblem::ResetTimeLimit()
{
  SCIPsetRealParam(scipSP_, "limits/time", 1e+20); // default 1e+20 s
}
/**
 * @brief Resets dynamic gap of the solution process to default value from Settings
 * 
 */
void SubProblem::ResetDynamicGap()
{
  dynamic_gap_ = Settings::kDynamicGap;
}

/**
 * @brief Create a binary Z_i variable
 * 
 * @param coil_i The coil i that the Z_i variable is created for
 */
void SubProblem::CreateZVariable(Coil coil_i)
{
  assert(vars_Z_.count(coil_i) == 0);
  char var_cons_name[Settings::kSCIPMaxStringLength];

  // create Z variable
  // initialize map entry, TODO: find out if this is necessary
  vars_Z_[coil_i] = nullptr;

  SCIP_VAR **z_var_pointer = &vars_Z_[coil_i];
  SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "Z_C%d", coil_i);
  SCIPcreateVarBasic(scipSP_,               //
                     z_var_pointer,         // returns the address of the newly created variable
                     var_cons_name,         // name
                     0,                     // lower bound
                     1,                     // upper bound
                     0,                     // objective function coefficient, equal to 0 according to model
                     SCIP_VARTYPE_INTEGER); // variable type

  SCIPaddVar(scipSP_, *z_var_pointer);
}

/**
 * @brief Create a  binary S_i variable
 * 
 * @param coil_i The coil i that the S_i variable is created for
 */
void SubProblem::CreateSVariable(Coil coil_i)
{
  assert(vars_S_.count(coil_i) == 0);
  // bounds of variable: if this is start coil, variable should be equal to 0, else 0 <= var <= +infty
  SCIP_Real lb = instance_->IsStartCoil(coil_i) ? 0 : 0;
  SCIP_Real ub = instance_->IsStartCoil(coil_i) ? 0 : SCIPinfinity(scipSP_);

  char var_cons_name[Settings::kSCIPMaxStringLength];

  // initialize variable ptr in map
  vars_S_[coil_i] = nullptr;

  SCIP_VAR **s_var_pointer = &vars_S_[coil_i];
  SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "S_C%d", coil_i);
  SCIPcreateVarBasic(scipSP_,                  //
                     s_var_pointer,            // returns the address of the newly created variable
                     var_cons_name,            // name
                     lb,                       // lower bound, see above
                     ub,                       // upper bound, see above
                     0,                        // objective function coefficient, equal to 0 according to model
                     SCIP_VARTYPE_CONTINUOUS); // variable type

  SCIPaddVar(scipSP_, *s_var_pointer);
}

/**
 * @brief Create a original binary X_ijkmn variable
 * 
 * @param coil_i Coil i this variable is created for
 * @param coil_j Coil j this variable is created for
 * @param line Production line this variable is created for
 * @param mode_i Mode m of coil i this variable is created for
 * @param mode_j Mode n of coil j this variable is created for
 */
void SubProblem::CreateXVariable(Coil coil_i, Coil coil_j, ProductionLine line, Mode mode_i, Mode mode_j)
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
  SCIPcreateVarBasic(scipSP_,               //
                     x_var_pointer,         // returns the address of the newly created variable
                     var_cons_name,         // name
                     lb,                    // lower bound
                     ub,                    // upper bound
                     0,                     // objective function coefficient, this is 0 since it is altered by pricer
                     SCIP_VARTYPE_INTEGER); // variable type

  SCIPaddVar(scipSP_, *x_var_pointer);
}

/**
 * @brief Checks if solution process of SCIP object was interrupted
 * 
 * @return true If solution process was interrupted
 * @return false If solution process was not interrupted
 */
bool SubProblem::WasInterrupted()
{
  return SCIPisSolveInterrupted(scipSP_);
}
/**
 * @brief Interrupts solution process of SCIP object
 * 
 */
void SubProblem::InterruptSolving()
{
  SCIPinterruptSolve(scipSP_);
}


// Do the work in Setup method instead of parameterless constructor
/**
 * @brief Setup the subproblem. Create SCIP object, set parameters, add variables, add constraints. 
 * 
 * @param instance The instance that is to be solved
 * @param line Production line of the subproblem
 */
void SubProblem::Setup(shared_ptr<Instance> instance, ProductionLine line)
{
  instance_ = instance;
  line_ = line;
  // first generate the Subproblem with the method of the compact Model
  SCIPcreate(&scipSP_);
  SCIPincludeDefaultPlugins(scipSP_);
  SCIPcreateProbBasic(scipSP_, "Subproblem PHALS");

  // set all optional SCIPParameters
  SCIPsetIntParam(scipSP_, "display/verblevel", 0);
  SCIPsetBoolParam(scipSP_, "display/lpinfo", FALSE);
  SCIPsetRealParam(scipSP_, "limits/time", 1e+20); // default 1e+20 s
  SCIPsetRealParam(scipSP_, "limits/gap", 0);      // default 0
  // instruct SCIP to collect incumbents
  SCIPsetBoolParam(scipSP_, "constraints/countsols/collect", TRUE);
  // we do not care about solutions, if these have a not negative optimal objfunc-value
  SCIPsetObjlimit(scipSP_, -SCIPepsilon(scipSP_));

  // enable reoptimization if wanted
  SCIPenableReoptimization(scipSP_, Settings::kEnableReoptimization);

  // create Helping-dummy for the name of variables and constraints
  char var_cons_name[Settings::kSCIPMaxStringLength];

  // #####################################################################################################################
  //  Create and add all variables
  // #####################################################################################################################

  // constant variable
  SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "Const");
  SCIPcreateVarBasic(scipSP_,                  //
                     &var_constant_one_,       // returns the address of the newly created variable
                     var_cons_name,            // name
                     1,                        // lower bound
                     1,                        // upper bound
                     0,                        // objective function coefficient, equal to 0 according to model
                     SCIP_VARTYPE_CONTINUOUS); // variable type

  SCIPaddVar(scipSP_, var_constant_one_);

  for (auto &coil_i : instance_->coilsWithoutEndCoil)
  {
    for (auto &coil_j : instance_->coilsWithoutStartCoil)
    {
      // if coil_i and coil_j are both sentinel coils, there should be no connection, thus skip this iteration,else, continue
      if (instance_->IsStartCoil(coil_i) && instance_->IsEndCoil(coil_j))
      {
        continue;
      }

      auto &modes_i = instance_->modes[make_tuple(coil_i, line_)];
      auto &modes_j = instance_->modes[make_tuple(coil_j, line_)];

      for (auto &mode_i : modes_i)
      {
        for (auto &mode_j : modes_j)
        {
          CreateXVariable(coil_i, coil_j, line_, mode_i, mode_j);
        }
      }
    }
  }
  // create other variables for coils
  for (auto &coil : instance_->coils)
  {
    this->CreateZVariable(coil);
    this->CreateSVariable(coil);
  }

  // #####################################################################################################################
  //  Add restrictions
  // #####################################################################################################################

  // (3) production line start: every line (this) has exactly one successor of starting coil
  // equivalent to 
  // 1 <= sum(regular coil coil_j, mode_i of start coil, mode_j of coil_j, X_(start coil),line of subproblem,coil_j,mode_i,mode_j <= 1
  SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "production_line_start");

  SCIPcreateConsBasicLinear(scipSP_,                      // scip
                            &cons_production_line_start_, // cons
                            var_cons_name,                // name
                            0,                            // nvar
                            0,                            // vars
                            0,                            // coeffs
                            1,                            // lhs
                            1);                           // rhs

  // add coefficients
  Coil coil_i = instance_->startCoil; // this is the start coil

  // skip sentinel coils, only regular coils
  for (auto &coil_j : instance_->regularCoils)
  {
    for (auto &mode_i : instance_->modes[make_tuple(coil_i, line_)])
    {
      for (auto &mode_j : instance_->modes[make_tuple(coil_j, line_)])
      {
        SCIPaddCoefLinear(scipSP_, cons_production_line_start_, vars_X_[make_tuple(coil_i, coil_j, line_, mode_i, mode_j)], 1);
      }
    }
  }
  SCIPaddCons(scipSP_, cons_production_line_start_);

  // (4) production line end: every line has exactly one predecessor of end coil
  // equivalent to 
  // 1 <= sum(regular coil coil_i, mode_i of coil_i, mode_j of end coil, X_coil_i,line of subproblem,end coil,mode_i,mode_j <= 1
 
  SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "production_line_end");

  SCIPcreateConsBasicLinear(scipSP_,                    // scip
                            &cons_production_line_end_, // cons
                            var_cons_name,              // name
                            0,                          // nvar
                            0,                          // vars
                            0,                          // coeffs
                            1,                          // lhs
                            1);                         // rhs

  // add coefficients
  Coil coil_j = instance_->endCoil; // this is the end coil
  // skip sentinel coils, only regular coils
  for (auto &coil_i : instance_->regularCoils)
  {
    for (auto &mode_i : instance_->modes[make_tuple(coil_i, line_)])
    {
      for (auto &mode_j : instance_->modes[make_tuple(coil_j, line_)])
      {
        auto tuple = make_tuple(coil_i, coil_j, line_, mode_i, mode_j);
        auto &var_X = vars_X_[tuple];
        SCIPaddCoefLinear(scipSP_, cons_production_line_end_, var_X, 1);
      }
    }
  }
  SCIPaddCons(scipSP_, cons_production_line_end_);

  // (5) flow conservation: for every line, every coil, and every corresponding mode there is equal amount of incoming and outgoing edges
  // see README
  // skip sentinel coils, only regular coils
  for (auto &coil_j : instance_->regularCoils)
  {

    auto coil_j_line_tuple = make_tuple(coil_j, line_);
    for (auto &mode_j : instance_->modes[coil_j_line_tuple])
    {
      auto cons_tuple = make_tuple(coil_j, mode_j);
      SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "cons_flow_conservation_C%d_M%d", coil_j, mode_j);

      SCIPcreateConsBasicLinear(scipSP_,                              // scip
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
        for (auto &mode_i : instance_->modes[make_tuple(coil_i, line_)])
        {
          SCIPaddCoefLinear(scipSP_, cons_flow_conservation_[cons_tuple], vars_X_[make_tuple(coil_i, coil_j, line_, mode_i, mode_j)], 1);
        }
      }

      // negative part of LHS: outgoing edges
      // skip start coil and include end coil
      for (auto &coil_i : instance_->coilsWithoutStartCoil)
      {
        if (instance_->IsStartCoil(coil_i))
          continue;

        for (auto &mode_i : instance_->modes[make_tuple(coil_i, line_)])
        {
          SCIPaddCoefLinear(scipSP_, cons_flow_conservation_[cons_tuple], vars_X_[make_tuple(coil_j, coil_i, line_, mode_j, mode_i)], -1);
        }
      }

      SCIPaddCons(scipSP_, cons_flow_conservation_[cons_tuple]);
    }
  }

  // (6) delay linking: link Z_i, S_i and X_..
  // skip non-regular coils
  // see compact model
  for (auto &coil_i : instance_->regularCoils)
  {
    SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "delay_linking_C%d", coil_i);

    SCIPcreateConsBasicLinear(scipSP_,                      // scip
                              &cons_delay_linking_[coil_i], // cons
                              var_cons_name,                // name
                              0,                            // nvar
                              0,                            // vars
                              0,                            // coeffs
                              -SCIPinfinity(scipSP_),       // lhs
                              0);                           // rhs

    // add coefficients
    // add S_i
    SCIPaddCoefLinear(scipSP_, cons_delay_linking_[coil_i], vars_S_[coil_i], 1);

    // other
    // if coil_j is start coil, skip
    for (auto &coil_j : instance_->coilsWithoutStartCoil)
    {
      for (auto &mode_j : instance_->modes[make_tuple(coil_j, line_)])
      {
        for (auto &mode_i : instance_->modes[make_tuple(coil_i, line_)])
        {
          auto processing_time = instance_->processingTimes[make_tuple(coil_i, line_, mode_i)];

          SCIPaddCoefLinear(scipSP_, cons_delay_linking_[coil_i], vars_X_[make_tuple(coil_i, coil_j, line_, mode_i, mode_j)], processing_time);
        }
      }
    }

    // RHS
    // due date d_i
    SCIPaddCoefLinear(scipSP_, cons_delay_linking_[coil_i], var_constant_one_, -instance_->dueDates[coil_i]);

    // big M linearization
    SCIP_Real big_M = instance_->bigM[line_];
    SCIPaddCoefLinear(scipSP_, cons_delay_linking_[coil_i], vars_Z_[coil_i], -big_M);

    SCIPaddCons(scipSP_, cons_delay_linking_[coil_i]);
  }

  // (7) start time linking: link S_i and X_..
  // skip non-regular coils for both coil_i and coil_j
  // see compact model
  for (auto &coil_i : instance_->regularCoils)
  {
    for (auto &coil_j : instance_->regularCoils)
    {

      auto con_tuple = make_tuple(coil_i, coil_j);

      SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "start_time_linking_CI%d_CJ%d", coil_i, coil_j);

      SCIPcreateConsBasicLinear(scipSP_,                              // scip
                                &cons_start_time_linking_[con_tuple], // cons
                                var_cons_name,                        // name
                                0,                                    // nvar
                                0,                                    // vars
                                0,                                    // coeffs
                                -SCIPinfinity(scipSP_),               // lhs
                                0);                                   // rhs

      // add coefficients
      // add S_i
      SCIPaddCoefLinear(scipSP_, cons_start_time_linking_[con_tuple], vars_S_[coil_i], 1);

      // add -S_j
      SCIPaddCoefLinear(scipSP_, cons_start_time_linking_[con_tuple], vars_S_[coil_j], -1);

      // add -M
      SCIP_Real big_M = instance_->bigM[line_];
      SCIPaddCoefLinear(scipSP_, cons_start_time_linking_[con_tuple], var_constant_one_, -big_M);
      for (auto &mode_i : instance_->modes[make_tuple(coil_i, line_)])
      {
        for (auto &mode_j : instance_->modes[make_tuple(coil_j, line_)])
        {
          // (p_ikm+tijkmn)*X_ijkmn
          auto processing_time = instance_->processingTimes[make_tuple(coil_i, line_, mode_i)];
          auto setup_time = instance_->setupTimes[make_tuple(coil_i, mode_i, coil_j, mode_j, line_)];
          SCIP_Real coefficient = processing_time + setup_time;

          SCIPaddCoefLinear(scipSP_, cons_start_time_linking_[con_tuple], vars_X_[make_tuple(coil_i, coil_j, line_, mode_i, mode_j)], coefficient);

          // M*X_ijkmn
          SCIPaddCoefLinear(scipSP_, cons_start_time_linking_[con_tuple], vars_X_[make_tuple(coil_i, coil_j, line_, mode_i, mode_j)], big_M);
        }
      }
      SCIPaddCons(scipSP_, cons_start_time_linking_[con_tuple]);
    }
  }

  // (8) max number of delayed columns
  // see README
  SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "max_delayed_coils");

  SCIPcreateConsBasicLinear(scipSP_,                         // scip
                            &cons_max_delayed_coils_,        // cons
                            var_cons_name,                   // name
                            0,                               // nvar
                            0,                               // vars
                            0,                               // coeffs
                            -SCIPinfinity(scipSP_),          // lhs
                            instance_->maximumDelayedCoils); // rhs
  // skip non-regular coils
  for (auto &coil_i : instance_->regularCoils)
  {
    SCIPaddCoefLinear(scipSP_, cons_max_delayed_coils_, vars_Z_[coil_i], 1);
  }

  SCIPaddCons(scipSP_, cons_max_delayed_coils_);

  // (TODO) delay linking: link sum(X_ijkmn) and Z_i, if Z_i is 1, at least one X_ijkmn must be 1
  // skip non-regular coils
  // for (auto &coil_i : instance_->regularCoils)
  // {
  //   SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "cons_delay_edge_linking_%d", coil_i);

  //   SCIPcreateConsBasicLinear(scipSP_,                           // scip
  //                             &cons_delay_edge_linking_[coil_i], // cons
  //                             var_cons_name,                     // name
  //                             0,                                 // nvar
  //                             0,                                 // vars
  //                             0,                                 // coeffs
  //                             -SCIPinfinity(scipSP_),            // lhs
  //                             0);                                // rhs

  //   // add coefficients
  //   // add Z_i
  //   SCIPaddCoefLinear(scipSP_, cons_delay_edge_linking_[coil_i], vars_Z_[coil_i], 1);

  //   // add <= sum(1*X_ijkmn) to constraint term
  //   for (auto const &[tuple, var_X] : vars_X_)
  //   {
  //     if (get<0>(tuple) == coil_i)
  //     {
  //       // if this variable incorporates coil_i as start point, add it to constraint
  //       SCIPaddCoefLinear(scipSP_, cons_delay_edge_linking_[coil_i], var_X, -1);
  //     }
  //   }
  //   SCIPaddCons(scipSP_, cons_delay_edge_linking_[coil_i]);
  // }
}
// destructor
/**
 * @brief Destroy the Sub Problem object and release SCIP ressources
 * 
 */
SubProblem::~SubProblem()
{
  SCIPreleaseCons(scipSP_, &cons_production_line_start_);
  SCIPreleaseCons(scipSP_, &cons_production_line_end_);
  SCIPreleaseCons(scipSP_, &cons_max_delayed_coils_);

  for (auto &[_, cons] : this->cons_flow_conservation_)
  {
    SCIPreleaseCons(scipSP_, &cons);
  }

  for (auto &[_, cons] : this->cons_delay_linking_)
  {
    SCIPreleaseCons(scipSP_, &cons);
  }

  for (auto &[_, cons] : this->cons_start_time_linking_)
  {
    SCIPreleaseCons(scipSP_, &cons);
  }

  for (auto &[_, cons] : this->cons_delay_edge_linking_)
  {
    SCIPreleaseCons(scipSP_, &cons);
  }

  for (auto &[_, var] : this->vars_X_)
  {
    SCIPreleaseVar(scipSP_, &var);
  }

  for (auto &[_, var] : this->vars_S_)
  {
    SCIPreleaseVar(scipSP_, &var);
  }

  for (auto &[_, var] : this->vars_Z_)
  {
    SCIPreleaseVar(scipSP_, &var);
  }

  SCIPreleaseVar(scipSP_, &var_constant_one_);

  SCIPfree(&scipSP_);
}

// update the objective-function of the Subproblem according to the new DualVariables with SCIPchgVarObj()
/**
 * @brief Updates the objective of the sub problem according to dual_values
 * 
 * @param dual_values Corresponding dual values: either Farkas multipliers or dual values of constraints of Master problem 
 * @param is_farkas If false, costs of pattern is part of objective, else not part of objective 
 */
void SubProblem::UpdateObjective(shared_ptr<DualValues> dual_values, const bool is_farkas)
{
  // if reoptimization is enabled, methods for freeing transformed problem and updating objective function are different

  // enable modifications
  if(Settings::kEnableReoptimization) {
    SCIPfreeReoptSolve(scipSP_); 
  } else {
    SCIPfreeTransform(scipSP_);
  }

  // collect all variables
  // estimate number of variables in objective
  auto nvars = instance_->coils.size() + vars_X_.size() + 1;

  // since SCIP needs contiguous memory of coefficients and variables for reoptimization objective function change,
  // collect coefficients in objective function
  vector<SCIP_Real> coeffs;

  // and1 collect variable pointers
  vector<SCIP_Var *> vars;

  // avoid unnecessary reallocations / resizings of vector by predefining size
  vars.resize(nvars);
  coeffs.resize(nvars);

  // current index of both "arrays"
  int i = 0;

  // loop through Z variables
  for (auto coil_i : instance_->coils)
  {
    // both delay coils constraint and original variable constraint duals need to be respected here
    // but only if coil_i is a regular coil, if not, don't consider dual of max delay constraint
    // Z occurs in original variable constraint with coefficient -1, thus coefficient in objective is --(1) = 1
    // coefficient: 
    // if regular coil: -pi_max_delayed_coils_ + pi_original_var_Z
    // if non-regular coil: pi_original_var_Z
    vars[i] = vars_Z_[coil_i];
    coeffs[i] = -(instance_->IsRegularCoil(coil_i) ? dual_values->pi_max_delayed_coils_ : 0) + dual_values->pi_original_var_Z[coil_i];
    i++;
  }

  // S variables don't occur in the MP, so no coefficients in reduced cost term

  // loop through every X_ijkmn variables
  for (auto &[tuple, var_X] : vars_X_)
  {
    // destructure
    auto &[coil_i, coil_j, line, mode_i, mode_j] = tuple;

    // this should always be the case, else, we have a problem
    assert(line == line_);

    // calculate coefficient of variable:
    // positive coefficient: original costs
    SCIP_Real column_cost = 0;

    // negative coefficients: dual costs introduced by constraints in MP
    SCIP_Real dual_cost = 0;
    // X_ijkmn variable occurs in original var constraint with coefficient -1, same argument as above
    SCIP_Real original_var_cost = dual_values->pi_original_var_X[tuple];

    // skip rest of cost if coil_i is non-regular since such variables do not occur at objective nor at coil partitioning constraint, only at original variable restore constraint
    if (instance_->IsRegularCoil(coil_i))
    {
      // set dual cost because coil_i always occurs in coil partitioning constraint
      dual_cost = -dual_values->pi_partitioning_[coil_i];

      // don't add column cost if coil_j is end coil since end coil only occurs at coil partitioning constraint and not in objective
      if (instance_->IsEndCoil(coil_j))
      {
        column_cost = 0;
      }
      else
      {
        column_cost = instance_->stringerCosts[make_tuple(coil_i, mode_i, coil_j, mode_j, line)];
      }

      // if we are doing Farkas pricing, don't use column cost in objective at all
      if (is_farkas)
        column_cost = 0;
    }

    vars[i] = var_X;
    // capture whole coefficient
    coeffs[i] = column_cost + dual_cost + original_var_cost;
    i++;
  }

  // convexity constraint for this line
  vars[i] = var_constant_one_;
  coeffs[i] = -dual_values->pi_convexity_[line_];
  i++;

  // this needs to hold because else our previous estimation of number of variables was wrong
  assert(i == nvars);

  if(Settings::kEnableReoptimization) {
    // since vectors are required to be contiguous memory, we can assume that pointer to first element is equivalent to C style array
    SCIPchgReoptObjective(scipSP_, SCIP_Objsense::SCIP_OBJSENSE_MINIMIZE, &vars[0], &coeffs[0], nvars);
  } else {
    // set coefficient for each variable
    for(int array_index = 0; array_index < nvars; array_index++) {
      SCIPchgVarObj(scipSP_, vars[array_index], coeffs[array_index]);
    }
  }
}

/**
 * @brief Trigger subproblem solving
 * 
 * @return vector<shared_ptr<ProductionLineSchedule>> Production Line schedules with negative reduced costs that were found 
 */
vector<shared_ptr<ProductionLineSchedule>> SubProblem::Solve()
{
  // capture schedules
  vector<shared_ptr<ProductionLineSchedule>> schedules;
  
  // set gap to specified dynamic gap
  this->SetGap(dynamic_gap_);
  
  char model_name[Settings::kSCIPMaxStringLength];
  (void)SCIPsnprintf(model_name, Settings::kSCIPMaxStringLength, "SubProblems/SubProblem_L%d_%d.lp", line_, iteration_);

  // write out to disk
  SCIPwriteOrigProblem(scipSP_, model_name, "lp", FALSE);

  // solve
  SCIPsolve(scipSP_);

  // restore solutions
  auto best_solution = SCIPgetBestSol(scipSP_);
  auto number_of_solutions = SCIPgetNSols(scipSP_);

  // return if no solutions found
  if (number_of_solutions == 0 || best_solution == NULL)
    return schedules;

  auto solutions = SCIPgetSols(scipSP_);
  // loop through every solution
  // explicitly include best solution by initializing solution_index to -1
  for (int solution_index = 0; solution_index < number_of_solutions; solution_index++)
  {

    // get solution from scip
    // if solution_index == -1, i.e. first iteration, take best solution
    auto scip_solution = solution_index == -1 ? best_solution : solutions[solution_index];

    // restore solution
    // allocate memory for schedule / column
    auto schedule = make_shared<ProductionLineSchedule>();

    // set line
    schedule->line = line_;

    // check if solution is null, i.e. no column was found
    if (scip_solution == NULL)
    {
      // don't add
      continue;
    }

    // get reduced cost of schedule
    schedule->reduced_cost = SCIPgetSolOrigObj(scipSP_, scip_solution);

    // check if reduced cost is negative
    // do it here because in pricing problem we need to access internal
    // -> coupling of pricer and subproblem, which should be abstracted away by common interface
    schedule->reduced_cost_negative = SCIPisNegative(scipSP_, schedule->reduced_cost + 0.001); // TODO: use scip methods here

    // calculate cost of schedule cost, i.e. coefficient of generated column in MP
    schedule->schedule_cost = 0;

    // furthermore, fill up incidences of schedule

    // iterate through every variable, and thus through every corresponding edge, and access variable identifier
    for (auto &[tuple, var_X] : vars_X_)
    {
      // restore var
      Coil coil_i = get<0>(tuple);
      Coil coil_j = get<1>(tuple);
      ProductionLine line = get<2>(tuple);
      Mode mode_i = get<3>(tuple);
      Mode mode_j = get<4>(tuple);

      // this should always be true
      assert(line == line_);

      // TODO: use SCIP epsilon methods here
      auto edge_selected = SCIPgetSolVal(scipSP_, scip_solution, var_X) > 0.5;

      // set incidence of this edge corresponding to current variable
      schedule->edges[tuple] = edge_selected;

      // add cost to schedule if edge is included, else don't
      if (edge_selected)
        schedule->schedule_cost += instance_->stringerCosts[make_tuple(coil_i, mode_i, coil_j, mode_j, line)];
    }

    // restore delayedness
    for (auto &[coil_i, var_Z] : vars_Z_)
    {
      auto coil_delayed = SCIPgetSolVal(scipSP_, scip_solution, var_Z) > 0.5; // TODO: use SCIP epsilon methods

      schedule->delayedness[coil_i] = coil_delayed;
    }

    // add solution to list
    schedules.push_back(schedule);
  }

  // increment iteration counter
  iteration_++;

  return schedules;
}

/**
 * @brief Gets dual bound of subproblem
 * 
 * @return SCIP_Real Dual bound of subproblem
 */
SCIP_Real SubProblem::GetDualBound()
{
  return SCIPgetDualbound(scipSP_);
}

/**
 * @brief Checks if dual bound is negative
 * 
 * @return true Dual bound is negative
 * @return false Dual bound is not negative
 */
bool SubProblem::IsDualBoundNegative()
{
  return SCIPisNegative(scipSP_, GetDualBound());
}