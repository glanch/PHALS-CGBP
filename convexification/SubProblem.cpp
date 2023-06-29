#include "SubProblem.h"
#include <memory>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>

SubProblem::SubProblem() {

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

  // TODO
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
    SCIPchgVarObj(scipSP_, vars_Z_[coil_i], -dual_values->pi_max_delayed_coils_);
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

    // skip cost if coil_i is non-regular since such variables do not occur at objective nor at coil partitioning constraint
    if(!instance_->IsRegularCoil(coil_i)) {
      continue;
    }


    // set dual cost because coil_i always occurs in coil partitioning constraint
    dual_cost = dual_values->pi_partitioning_[coil_i];

    // don't add column cost if coil_j is end coil since end coil only occurs at coil partitioning constraint
    if (instance_->IsEndCoil(coil_j))
    { 
      column_cost = 0;
    } else {
      column_cost = instance_->stringerCosts[make_tuple(coil_i, mode_i, coil_j, mode_j, line)];
    }

    // if we are doing Farkas pricing, don't use column cost in objective at all
    if(is_farkas)
      column_cost = 0;

    SCIPchgVarObj(scipSP_, var_X, column_cost - dual_cost);
  }
  
  // convexity constraint for this line
  SCIPchgVarObj(scipSP_, var_constant_one_, -dual_values->pi_convexity_[line_]);
}

shared_ptr<ProductionLineSchedule> SubProblem::Solve()
{
  char model_name[Settings::kSCIPMaxStringLength];
  (void)SCIPsnprintf(model_name, Settings::kSCIPMaxStringLength, "SubProblem_L%d.lp", line_);

  // write out to disk
  SCIPwriteOrigProblem(scipSP_, model_name, "lp", FALSE);

  // solve
  SCIPsolve(scipSP_);

  // restore solution
  // allocate memory for schedule / column
  auto schedule = make_shared<ProductionLineSchedule>();

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

  return schedule;
}
