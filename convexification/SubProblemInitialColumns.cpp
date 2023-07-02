#include "SubProblemInitialColumns.h"
#include <memory>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>

bool SubProblemInitialColumns::RemoveMinCoilCountIfExists()
{
  // if cons ptr is null, the constraint hasn't been created
  // else, we remove it from problem, free it and set pointer of cons to nullptr
  if (cons_min_coils_ != nullptr)
  {
    SCIPdelCons(scipSP_, cons_min_coils_);
    SCIPreleaseCons(scipSP_, &cons_min_coils_);

    cons_min_coils_ = nullptr;
    return true;
  }
  else
  {
    return false;
  }
}
void SubProblemInitialColumns::SetMinCoilCount(int count)
{
  // if cons ptr is null, the constraint hasn't been created
  // else, we remove it from problem and free it, afterwards create a new constraint and add it to problem
  RemoveMinCoilCountIfExists();

  // create new constraint
  CreateMinCoilCountConstraint(count);
  SCIPaddCons(scipSP_, cons_min_coils_);
}

bool SubProblemInitialColumns::RemoveMaxCoilCountIfExists()
{
  // if cons ptr is null, the constraint hasn't been created
  // else, we remove it from problem, free it and set pointer of cons to nullptr
  if (cons_max_coils_ != nullptr)
  {
    SCIPdelCons(scipSP_, cons_max_coils_);
    SCIPreleaseCons(scipSP_, &cons_max_coils_);

    cons_max_coils_ = nullptr;
    return true;
  }
  else
  {
    return false;
  }
}
void SubProblemInitialColumns::SetMaxCoilCount(int count)
{
  // if cons ptr is null, the constraint hasn't been created
  // else, we remove it from problem and free it, afterwards create a new constraint and add it to problem
  RemoveMaxCoilCountIfExists();

  // create new constraint
  CreateMaxCoilCountConstraint(count);
  SCIPaddCons(scipSP_, cons_max_coils_);
}

void SubProblemInitialColumns::EnableInScheduleConstraint(Coil coil)
{
  // check if constraint already created
  // if yes, we can continue by enabling the constraint, else create it
  if (cons_coil_in_schedule_.count(coil))
  {
    // if not existent, create constraint
    CreateInScheduleConstraint(coil);
  }

  // now add constraint to scip, thus enabling the constraint
  SCIPaddCons(scipSP_, cons_coil_in_schedule_[coil]);
}

void SubProblemInitialColumns::DisableInScheduleConstraint(Coil coil)
{
  // check if constraint already present. if this is the case, remove it from
  // if it is not present, nothing to do here
  if (cons_coil_in_schedule_.count(coil))
  {
    // delete the constraint
    SCIPdelCons(scipSP_, cons_coil_in_schedule_[coil]);
  }
}

void SubProblemInitialColumns::EnableOutScheduleConstraint(Coil coil)
{
  // check if constraint already created
  // if yes, we can continue by enabling the constraint, else create it
  if (cons_coil_out_schedule_.count(coil))
  {
    // if not existent, create constraint
    CreateOutScheduleConstraint(coil);
  }

  // now add constraint to scip, thus enabling the constraint
  SCIPaddCons(scipSP_, cons_coil_out_schedule_[coil]);
}

void SubProblemInitialColumns::DisableOutScheduleConstraint(Coil coil)
{
  // check if constraint already present. if this is the case, remove it from
  // if it is not present, nothing to do here
  if (cons_coil_out_schedule_.count(coil))
  {
    // delete the constraint
    SCIPdelCons(scipSP_, cons_coil_out_schedule_[coil]);
  }
}

void SubProblemInitialColumns::CreateInScheduleConstraint(Coil coil)
{
  char var_cons_name[Settings::kSCIPMaxStringLength];

  SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "coil_in_schedule_%d", coil);

  SCIPcreateConsBasicLinear(scipSP_,                       // scip
                            &cons_coil_in_schedule_[coil], // cons
                            var_cons_name,                 // name
                            0,                             // nvar
                            0,                             // vars
                            0,                             // coeffs
                            1,                             // lhs
                            1);                            // rhs

  // add all variables X_ijkmn, whereas i is equal to passed parameter coil
  // thus enforcing that there is exactly one edge outgoing from coil coil
  for (auto &[tuple, var] : vars_X_)
  {
    auto &[coil_i, coil_j, line_, mode_i, mode_j] = tuple;

    if (coil_i == coil)
    {
      SCIPaddCoefLinear(scipSP_, cons_coil_in_schedule_[coil], var, 1);
    }
  }
}

void SubProblemInitialColumns::CreateOutScheduleConstraint(Coil coil)
{
  char var_cons_name[Settings::kSCIPMaxStringLength];

  SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "coil_out_schedule_%d", coil);

  SCIPcreateConsBasicLinear(scipSP_,                        // scip
                            &cons_coil_out_schedule_[coil], // cons
                            var_cons_name,                  // name
                            0,                              // nvar
                            0,                              // vars
                            0,                              // coeffs
                            0,                              // lhs
                            0);                             // rhs

  // add all variables X_ijkmn, whereas i is equal to passed parameter coil
  // thus enforcing that there is exactly one edge outgoing from coil coil
  for (auto &[tuple, var] : vars_X_)
  {
    auto &[coil_i, coil_j, line_, mode_i, mode_j] = tuple;

    if (coil_i == coil)
    {
      SCIPaddCoefLinear(scipSP_, cons_coil_out_schedule_[coil], var, 1);
    }
  }
}

void SubProblemInitialColumns::CreateMinCoilCountConstraint(int count)
{
  // it should be the nullptr
  assert(cons_min_coils_ == nullptr);

  char var_cons_name[Settings::kSCIPMaxStringLength];

  SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "min_coil_count");

  SCIPcreateConsBasicLinear(scipSP_,                // scip
                            &cons_min_coils_,       // cons
                            var_cons_name,          // name
                            0,                      // nvar
                            0,                      // vars
                            0,                      // coeffs
                            count,                  // lhs
                            SCIPinfinity(scipSP_)); // rhs

  // add all variables X_ijkmn, without i being a start column
  for (auto &[tuple, var] : vars_X_)
  {
    auto &[coil_i, coil_j, line_, mode_i, mode_j] = tuple;

    if (instance_->IsRegularCoil(coil_i))
    {
      SCIPaddCoefLinear(scipSP_, cons_min_coils_, var, 1);
    }
  }
}

void SubProblemInitialColumns::CreateMaxCoilCountConstraint(int count)
{
  // it should be the nullptr
  assert(cons_max_coils_ == nullptr);

  char var_cons_name[Settings::kSCIPMaxStringLength];

  SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "max_coil_count");

  SCIPcreateConsBasicLinear(scipSP_,                // scip
                            &cons_max_coils_,       // cons
                            var_cons_name,          // name
                            0,                      // nvar
                            0,                      // vars
                            0,                      // coeffs
                            -SCIPinfinity(scipSP_), // lhs
                            count);                 // rhs

  // add all variables X_ijkmn, without i being a start column
  for (auto &[tuple, var] : vars_X_)
  {
    auto &[coil_i, coil_j, line_, mode_i, mode_j] = tuple;

    if (instance_->IsRegularCoil(coil_i))
    {
      SCIPaddCoefLinear(scipSP_, cons_max_coils_, var, 1);
    }
  }
}

void SubProblemInitialColumns::SetGap(double gap)
{
  gap_ = gap;
  SCIPsetRealParam(scipSP_, "limits/gap", gap_); // default 0
}

void SubProblemInitialColumns::ResetDynamicGap()
{
  dynamic_gap_ = Settings::kDynamicGap;
}
void SubProblemInitialColumns::CreateZVariable(Coil coil_i)
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
                     0,                     // objective value
                     SCIP_VARTYPE_INTEGER); // variable type

  SCIPaddVar(scipSP_, *z_var_pointer);
}

void SubProblemInitialColumns::CreateSVariable(Coil coil_i)
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
void SubProblemInitialColumns::CreateXVariable(Coil coil_i, Coil coil_j, ProductionLine line, Mode mode_i, Mode mode_j)
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
                     -1,                    // objective function coefficient, equal to -1 because we want to maximize the amount of selected nodes
                     SCIP_VARTYPE_INTEGER); // variable type

  SCIPaddVar(scipSP_, *x_var_pointer);
}

SubProblemInitialColumns::SubProblemInitialColumns() : cons_max_coils_(nullptr), cons_min_coils_(nullptr)
{
}
// Do the work in Setup method instead of parameterless constructor
void SubProblemInitialColumns::Setup(shared_ptr<Instance> instance, ProductionLine line)
{
  instance_ = instance;
  line_ = line;
  // first generate the Subproblem with the method of the compact Model
  SCIPcreate(&scipSP_);
  SCIPincludeDefaultPlugins(scipSP_);
  SCIPcreateProbBasic(scipSP_, "Subproblem PHALS Initial Column Search");

  // set all optional SCIPParameters
  SCIPsetIntParam(scipSP_, "display/verblevel", 0);
  SCIPsetBoolParam(scipSP_, "display/lpinfo", FALSE);

  // we do not care about objective of solution

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
      // if (coil_i != coil_j)
      // {
      // TODO: check this!
      auto &modes_i = instance_->modes[make_tuple(coil_i, line_)];
      auto &modes_j = instance_->modes[make_tuple(coil_j, line_)];

      for (auto &mode_i : modes_i)
      {
        for (auto &mode_j : modes_j)
        {
          CreateXVariable(coil_i, coil_j, line_, mode_i, mode_j);
        }
      }
      // }
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

  // (3) production line start: every line (this) has exactly one successor of starting coil
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

  // // (8) max number of delayed columns
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
  for (auto &coil_i : instance_->regularCoils)
  {
    SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "cons_delay_edge_linking_%d", coil_i);

    SCIPcreateConsBasicLinear(scipSP_,                           // scip
                              &cons_delay_edge_linking_[coil_i], // cons
                              var_cons_name,                     // name
                              0,                                 // nvar
                              0,                                 // vars
                              0,                                 // coeffs
                              -SCIPinfinity(scipSP_),            // lhs
                              0);                                // rhs

    // add coefficients
    // add Z_i
    SCIPaddCoefLinear(scipSP_, cons_delay_edge_linking_[coil_i], vars_Z_[coil_i], 1);

    // add <= sum(1*X_ijkmn) to constraint term
    for (auto const &[tuple, var_X] : vars_X_)
    {
      if (get<0>(tuple) == coil_i)
      {
        // if this variable incorporates coil_i as start point, add it to constraint
        SCIPaddCoefLinear(scipSP_, cons_delay_edge_linking_[coil_i], var_X, -1);
      }
    }
    SCIPaddCons(scipSP_, cons_delay_edge_linking_[coil_i]);
  }
}
// destructor
SubProblemInitialColumns::~SubProblemInitialColumns()
{
  SCIPreleaseCons(scipSP_, &cons_production_line_start_);
  SCIPreleaseCons(scipSP_, &cons_production_line_end_);

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

  // release "lazy" constraints
  for (auto &[_, cons] : this->cons_coil_in_schedule_)
  {
    SCIPreleaseCons(scipSP_, &cons);
  }

  for (auto &[_, cons] : this->cons_coil_out_schedule_)
  {
    SCIPreleaseCons(scipSP_, &cons);
  }

  if (cons_min_coils_ != nullptr)
  {
    SCIPreleaseCons(scipSP_, &cons_min_coils_);
  }

  // release "lazy" constraints
  if (cons_min_coils_ != nullptr)
  {
    SCIPreleaseCons(scipSP_, &cons_max_coils_);
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
void SubProblemInitialColumns::UpdateObjective(shared_ptr<DualValues> dual_values, const bool is_farkas)
{
  // don't do anything else beside allowing changes of subproblem
  SCIPfreeTransform(scipSP_);
}

shared_ptr<ProductionLineSchedule> SubProblemInitialColumns::Solve()
{

  cout << "Solving Initial Column Subproblem for line " << line_ << " with dynamic gap " << dynamic_gap_ << endl;
  
  // TODO: tune this
  this->SetGap(0.5);
  SCIPsetRealParam(scipSP_, "limits/time", 60);

  char model_name[Settings::kSCIPMaxStringLength];
  (void)SCIPsnprintf(model_name, Settings::kSCIPMaxStringLength, "SubProblems/InitialColumns_SubProblem_L%d_%d.lp", line_, iteration_);

  // write out to disk
  SCIPwriteOrigProblem(scipSP_, model_name, "lp", FALSE);

  // solve
  SCIPsolve(scipSP_);

  // restore solution
  // allocate memory for schedule / column
  auto schedule = make_shared<ProductionLineSchedule>();

  // set line
  schedule->line = line_;

  // get best solution
  SCIP_SOL *scip_solution = SCIPgetBestSol(scipSP_);

  // check if solution is null, i.e. no column was found
  if (scip_solution == NULL)
  {
    schedule->reduced_cost = 0;
    schedule->reduced_cost_negative = false;
    return schedule;
  }

  // if column is feasible, add it to master problem, even though the term "reduced cost" do not apply here
  schedule->reduced_cost_negative = true;

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

  iteration_++;
  return schedule;
}
