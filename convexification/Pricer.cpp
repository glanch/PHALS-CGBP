#include <memory>
#include "Pricer.h"

using namespace std;
using namespace scip;

MyPricer::MyPricer(shared_ptr<Master> master_problem, const char *pricer_name, const char *pricer_desc, int pricer_priority, SCIP_Bool pricer_delay)
    : ObjPricer(master_problem->scipRMP_, pricer_name, pricer_desc, pricer_priority, pricer_delay), // TRUE : LP is re-optimized each time a variable is added
      pricer_name_(pricer_name), pricer_desc_(pricer_desc), master_problem_(master_problem), scipRMP_(master_problem->scipRMP_), instance_(master_problem->instance_)
{
  // Initialize dual values object
  this->dual_values_ = make_shared<DualValues>(instance_);

  // Initialize each subproblem
  for (auto &line : instance_->productionLines)
  {
    this->subproblems_[line].Setup(instance_, line);
  }
}

MyPricer::~MyPricer()
{
  // neither subproblems nor dual variables need to be explicitly destroyed because we used smart pointers
}

/**
 * @brief get the pointers to the variables and constraints of the transformed problem
 *
 * @param scip
 * @param pricer
 * @return SCIP_RETCODE
 *
 * @note This code is part of a SCIP plugin and it initializes the plugin's pricer. In particular, it obtains pointers
 * to the variables and constraints of the transformed problem.
 * First, the code tries to retrieve the transformed variables. However, since there are no variables in the transformed
 * problem yet, this part of the code is commented out.
 * Next, the code retrieves the transformed constraints. It does this by looping through each item
 * and using the SCIPgetTransformedCons function to get the transformed constraint corresponding to the
 * onePatternPerItem constraint. Finally, the code returns SCIP_OKAY to indicate that the
 * initialization was successful. Overall, this code is responsible for retrieving the transformed constraints so that
 * the plugin can modify them as needed using the addNewVar() function.
 */
SCIP_RETCODE MyPricer::scip_init(SCIP *scip, SCIP_PRICER *pricer)
{
  // get transformed variables
  // X
  for (auto &[tuple, _] : master_problem_->vars_X_)
  {
    SCIPgetTransformedVar(scipRMP_, master_problem_->vars_X_[tuple], &(master_problem_->vars_X_[tuple]));
  }

  // Z
  for (auto &[coil, _] : master_problem_->vars_Z_)
  {
    SCIPgetTransformedVar(scipRMP_, master_problem_->vars_Z_[coil], &(master_problem_->vars_Z_[coil]));
  }

  // S
  for (auto &[coil, _] : master_problem_->vars_S_)
  {
    SCIPgetTransformedVar(scipRMP_, master_problem_->vars_S_[coil], &(master_problem_->vars_S_[coil]));
  }

  // ###########################################################################################################
  // get transformed constraints:
  // to get all transformed constraints, use the same loops, as in generation in the master-problem
  // ###########################################################################################################

  // coil partitioning
  for (auto &[tuple, _] : master_problem_->cons_coil_partitioning_)
  {
    SCIPgetTransformedCons(scipRMP_, master_problem_->cons_coil_partitioning_[tuple], &(master_problem_->cons_coil_partitioning_[tuple]));
  }

  // max delayed constraint
  SCIPgetTransformedCons(scipRMP_, master_problem_->cons_max_delayed_coils_, &master_problem_->cons_max_delayed_coils_);

  // convexity constraints
  for (auto &[tuple, _] : master_problem_->cons_convexity_)
  {
    SCIPgetTransformedCons(scipRMP_, master_problem_->cons_convexity_[tuple], &(master_problem_->cons_convexity_[tuple]));
  }

  // original variable restoring
  // X
  for (auto &[tuple, _] : master_problem_->cons_original_var_X)
  {
    SCIPgetTransformedCons(scipRMP_, master_problem_->cons_original_var_X[tuple], &(master_problem_->cons_original_var_X[tuple]));
  }

  // Z
  for (auto &[coil, _] : master_problem_->cons_original_var_Z)
  {
    SCIPgetTransformedCons(scipRMP_, master_problem_->cons_original_var_Z[coil], &(master_problem_->cons_original_var_Z[coil]));
  }

  // S
  for (auto &[coil, _] : master_problem_->cons_original_var_S)
  {
    SCIPgetTransformedCons(scipRMP_, master_problem_->cons_original_var_S[coil], &(master_problem_->cons_original_var_S[coil]));
  }

  return SCIP_OKAY;
}

/**
 * @brief perform pricing for dual and farkas combined with Flag isFarkas
 *
 * @param isFarkas perform farkas whether the master problem is LP-infeasible
 *
 * @return SCIP_RESULT
 */
SCIP_RESULT MyPricer::Pricing(const bool is_farkas)
{
  // ############################################################################################################
  //  define the dual variables
  //  we need one value for every constraint, so use the same loops as in the master-problem
  //  is isFarkas == true, then use the farkas-multipliers, is isFarkas == false, use the dual-variables
  //  After getting a value, check, if the value is possible, for <= or >= - constraints, we know the sign, so we can
  //  force them. We do this to avoid numerical-issues
  // ############################################################################################################

  // partitioning constraint
  for (auto &[coil, cons] : master_problem_->cons_coil_partitioning_)
  {
    dual_values_->pi_partitioning_[coil] = is_farkas ? SCIPgetDualfarkasLinear(scipRMP_, cons)
                                                     : SCIPgetDualsolLinear(scipRMP_, cons);
  }

  // convexity constraint
  for (auto &[line, cons] : master_problem_->cons_convexity_)
  {
    dual_values_->pi_convexity_[line] = is_farkas ? SCIPgetDualfarkasLinear(scipRMP_, cons)
                                                  : SCIPgetDualsolLinear(scipRMP_, cons);
  }

  // delayed coils constraint
  dual_values_->pi_max_delayed_coils_ = is_farkas ? SCIPgetDualfarkasLinear(scipRMP_, master_problem_->cons_max_delayed_coils_)
                                                  : SCIPgetDualsolLinear(scipRMP_, master_problem_->cons_max_delayed_coils_);

  // Update objective function of and solve all subproblems
  for (auto &[line, subproblem] : this->subproblems_)
  {
    subproblem.UpdateObjective(dual_values_, is_farkas);
    auto subproblem_solution = subproblem.Solve();

    // add variable if it could happen that selecting it improves the objective
    if (subproblem_solution->reduced_cost_negative) {
      DisplaySchedule(subproblem_solution);
      AddNewVar(subproblem_solution);
    }
  }

  // TODO: check this
  return SCIP_SUCCESS;
}

/**
 * @brief perform dual-pricing
 *
 * @param scip  an instance of the SCIP solver
 * @param pricer an instance of the SCIP pricer
 * @param lowerbound  a pointer to store the resulting lower bound
 * @param stopearly a pointer to signal whether to stop the pricing process early
 * @param result a pointer to store the SCIP result
 * @return SCIP_RETCODE
 *
 * @note calls the "pricing" method of the "MyPricer" class, passing in a boolean value of false, and stores the
 * resulting SCIP result in the "result" pointer.
 */
SCIP_RETCODE MyPricer::scip_redcost(SCIP *scip,
                                    SCIP_PRICER *pricer,
                                    SCIP_Real *lowerbound,
                                    SCIP_Bool *stopearly,
                                    SCIP_RESULT *result)
{
  cout << "Dual-Pricing: ";
  // start dual-pricing with isFarkas-Flag = false
  *result = Pricing(false);

  cout << endl;
  return SCIP_OKAY;
}

/**
 * @brief perform farkas-pricing
 *
 * @param scip  an instance of the SCIP solver
 * @param pricer an instance of the SCIP pricer
 * @param result a pointer to store the SCIP result
 * @return SCIP_RETCODE
 *
 * @note calls the "pricing" method of the "MyPricer" class, passing in a boolean value of true, and stores the
 * resulting SCIP result in the "result" pointer.
 */
SCIP_RETCODE MyPricer::scip_farkas(SCIP *scip, SCIP_PRICER *pricer, SCIP_RESULT *result)
{

  cout << "Farkas-Pricing: ";
  // start dual-pricing with is_farkas-Flag = false
  *result = Pricing(true);

  cout << endl;

  return SCIP_OKAY;
}

/**
 * @brief add a new variable (a new possible pattern for a bin) to the master problem.
 *
 * @param solution a pointer to a solution of the subproblem
 *
 * @note  a function addNewVar in the MyPricer class that adds a new variable to the SCIP optimization model. The
 * variable represents a packing pattern of a bin and is added to the master problem. The function creates a new
 * variable and assigns a unique name to it. It then sets the lower and upper bounds of the variable and its objective
 * value in the model. The function adds the newly created variable as a priced variable to the SCIP optimization model.
 * The function then adds coefficients to the one (non-dummy) constraint in the model: onePatternPerItem. It
 * iterates through each item and adds the corresponding coefficient in the onePatternPerItem constraint. The newly
 * created variable is also added to a list of lambdas in the _pbMaster object. Finally, the function writes the updated
 * optimization model to a file for inspection.
 */
void MyPricer::AddNewVar(shared_ptr<ProductionLineSchedule> schedule)
{
  auto line = schedule->line;

  char var_name[Settings::kSCIPMaxStringLength];

  // create the new variable
  SCIP_VAR *new_variable;

  auto lambda_index = master_problem_->vars_lambda_[schedule->line].size();

  schedule->lambda_index = lambda_index;

  (void)SCIPsnprintf(var_name, Settings::kSCIPMaxStringLength, "lambda_L%d_%d", schedule->line, schedule->lambda_index); // create name

  SCIPcreateVar(scipRMP_,                // scip-env
                &new_variable,           // connect with the new variable
                var_name,                // set name
                0.0,                     // lower bound
                SCIPinfinity(scipRMP_),  // upper bound
                schedule->schedule_cost, // objective
                SCIP_VARTYPE_CONTINUOUS, // continouus since we are using convexification
                false,
                false,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL);

  // add the new variable and resume the simplex-algorithm with the reducedCosts
  // TODO: find out why this is negative here
  SCIPaddPricedVar(scipRMP_, new_variable, -schedule->reduced_cost);

  // add variable to list of lambda variable per line
  master_problem_->vars_lambda_[schedule->line].push_back(new_variable);

  // add schedule to list of schedules per line
  master_problem_->schedules_[schedule->line].push_back(schedule);

  // ############################################################################################################

  //  add coefficients to the constraints

  // partitioning constraint
  // TODO: rethink this loop since it is not efficient, at least not as it could be ;-)
  for (auto coil_i : instance_->coilsWithoutStartCoil)
  {
    for (auto &[tuple, incidence] : schedule->edges)
    {
      const auto &[tuple_coil_i, tuple_coil_j, tuple_line, tuple_mode_i, tuple_mode_j] = tuple;

      if (tuple_coil_i != coil_i)
      {
        // skip this variable since it does not occur in this constraint
        continue;
      }

      // TODO: decide whether to add to constraint if incidence == false
      SCIP_Real column_coefficient = incidence ? 1 : 0;
      SCIPaddCoefLinear(scipRMP_, master_problem_->cons_original_var_X[tuple], new_variable, column_coefficient);
    }
  }

  // max delay constraint
  for (auto &[coil, delayed] : schedule->delayedness)
  {
    // TODO: decide whether to add if incidence == false
    SCIP_Real column_coefficient = delayed ? 1 : 0;
    SCIPaddCoefLinear(scipRMP_, master_problem_->cons_max_delayed_coils_, new_variable, column_coefficient);
  }

  // convexity constraint
  SCIPaddCoefLinear(scipRMP_, master_problem_->cons_convexity_[schedule->line], new_variable, 1);

  // original variable reconstruction
  // var X
  for (auto &[tuple, incidence] : schedule->edges)
  {
    // TODO: decide whether to add to constraint if incidence == false
    SCIP_Real column_coefficient = incidence ? 1 : 0;
    SCIPaddCoefLinear(scipRMP_, master_problem_->cons_original_var_X[tuple], new_variable, column_coefficient);
  }

  // var Z
  for (auto &[coil, delayed] : schedule->delayedness)
  {
    // TODO: decide whether to add if incidence == false
    SCIP_Real column_coefficient = delayed ? 1 : 0;
    SCIPaddCoefLinear(scipRMP_, master_problem_->cons_original_var_Z[coil], new_variable, column_coefficient);
  }

  char model_name[Settings::kSCIPMaxStringLength];
  (void)SCIPsnprintf(model_name, Settings::kSCIPMaxStringLength, "TransMasterProblems/TransMaster_%d_%d.lp", schedule->line, lambda_index);
  SCIPwriteTransProblem(scipRMP_, model_name, "lp", FALSE);
}
/** @brief
 * @param solution The solution that should be printed
 * @note print the results of a calculation to the console. It takes the solution of SubProblem::solve() and prints it
 * to the console, along with some additional text. The function prints the reduced
 * and newly generated packing pattern costs. It then iterates through the PatternIncidence boolean vector and prints the
 * indices where the value is true, thus displaying whether an item is part of the new pattern.
 */
void MyPricer::DisplaySchedule(shared_ptr<ProductionLineSchedule> column)
{
  cout << "Schedule / new column" << endl
       << "\tLine L" << column->line << endl
       << "\tReduced costs: " << column->reduced_cost << endl
       << "\tSchedule costs: " << column->schedule_cost << endl
       << "\tEdges: ";
  for (auto &[tuple, incidence] : column->edges)
  {
    auto &[coil_i, coil_j, line, mode_i, mode_j] = tuple;
    assert(line == column->line);

    if (incidence)
      cout << "\t\tC" << coil_i << "M" << mode_i << " -> C" << coil_j << "M" << mode_j << endl;
  }

  cout << "\tDelayed coils:" << endl;
  for (auto &[coil, delayed] : column->delayedness)
  {
    if (delayed)
      cout << "\t\tCoil " << coil << endl;
  }

  cout << endl;
}