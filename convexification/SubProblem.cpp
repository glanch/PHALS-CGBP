#include "SubProblem.h"
#include <memory>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>

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

SubProblem::SubProblem()
{
}
// Do the work in Setup method instead of parameterless constructor
void SubProblem::Setup(shared_ptr<Instance> instance, ProductionLine line)
{
  instance_ = instance;
  line_ = line;
  // first generate the Subproblem with the method of the compact Model
  SCIPcreate(&scipSP_);
  SCIPincludeDefaultPlugins(scipSP_);
  SCIPcreateProbBasic(scipSP_, "Subproblem BPP");

  // set all optional SCIPParameters
  SCIPsetIntParam(scipSP_, "display/verblevel", 0);
  SCIPsetBoolParam(scipSP_, "display/lpinfo", FALSE);

  // we do not care about solutions, if these have a not negative optimal objfunc-value
  SCIPsetObjlimit(scipSP_, -SCIPepsilon(scipSP_));

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
    SCIP_Real big_M = Settings::kBigM; // TODO: !!!!
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
      SCIP_Real big_M = Settings::kBigM;
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
}
// destructor
SubProblem::~SubProblem()
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
void SubProblem::UpdateObjective(shared_ptr<DualValues> dual_values, const bool is_farkas)
{
  SCIPfreeTransform(scipSP_); // enable modifications

  // Z variables
  for (auto coil_i : instance_->coils)
  {
    // both delay coils constraint and original variable constraint duals need to be respected here
    // but only if coil_i is a regular coil, if not, don't consider dual of max delay constraint
    SCIPchgVarObj(scipSP_, vars_Z_[coil_i], -((instance_->IsRegularCoil(coil_i) ? dual_values->pi_max_delayed_coils_ : 0) + dual_values->pi_original_var_Z[coil_i]));
  }

  // S variables don't occur in the MP, so no coefficients in reduced cost term

  // X_ijkmn variables
  for (auto &[tuple, var_X] : vars_X_)
  {
    // destructure
    auto &[coil_i, coil_j, line, mode_i, mode_j] = tuple;
    // this should always be the case, else, we have a problem
    assert(line == line_);

    SCIP_Real column_cost = 0;
    SCIP_Real dual_cost = 0;
    SCIP_Real original_var_cost = dual_values->pi_original_var_X[tuple];

    // skip rest of cost if coil_i is non-regular since such variables do not occur at objective nor at coil partitioning constraint, only at original variable restore constraint
    if (instance_->IsRegularCoil(coil_i))
    {

      // set dual cost because coil_i always occurs in coil partitioning constraint
      dual_cost = dual_values->pi_partitioning_[coil_i];

      // don't add column cost if coil_j is end coil since end coil only occurs at coil partitioning constraint
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
    SCIPchgVarObj(scipSP_, var_X, column_cost - dual_cost - original_var_cost);
  }

  // convexity constraint for this line
  SCIPchgVarObj(scipSP_, var_constant_one_, -dual_values->pi_convexity_[line_]);
}

shared_ptr<ProductionLineSchedule> SubProblem::Solve()
{

  char model_name[Settings::kSCIPMaxStringLength];
  (void)SCIPsnprintf(model_name, Settings::kSCIPMaxStringLength, "SubProblems/SubProblem_L%d_%d.lp", line_, iteration_);

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

  iteration_++;
  return schedule;
}
