#include "Master.h"
#include <scip/scip_cons.h>

/**
 * @brief Construct a new Master:: Master object
 *
 * @param ins pointer to a instance object
 *
 * @note This code defines the constructor for the Master class, which is part of a larger program for solving a BPP
 * using column generation.
 * The constructor takes an instance object (ins) as input and assigns it to a private pointer variable (_ins) in the
 * Master class.
 * It then creates a SCIP (Solving Constraint Integer Programs) environment and loads all the default plugins.
 * After that, it creates an empty problem using the SCIPcreateProb() function.
 * Next, it sets all optional SCIP parameters by calling the setSCIPParameters() function.
 * Following this, the function creates the name dummy variable (var_cons_name), resizes a vector (_var_lambda) for use
 * later, and creates one set of linear constraints: _cons_onePatternPerItem
 * The _cons_onePatternPerItem constraints ensure that each item is packed is packed exactly once.
 * Finally, the function writes the original LP program to a file using the SCIPwriteOrigProblem() function
 */
Master::Master(shared_ptr<Instance> instance) : instance_(instance)
{
   // create a SCIP environment and load all defaults
   SCIPcreate(&scipRMP_);
   SCIPincludeDefaultPlugins(scipRMP_);

   // create an empty problem
   SCIPcreateProb(scipRMP_, "master-problem BPP", 0, 0, 0, 0, 0, 0, 0);

   // set all optional SCIPParameters
   setSCIPParameters();

   // create Helping-dummy for the name of variables and constraints
   char var_cons_name[255];

   // add variables:
   // we have currently no variables, cause we are at the beginning of our columnGeneration-Process.
   _var_lambda.resize(0);

   // #####################################################################################
   // add restrictions with SCIPcreateConsLinear(), it needs to be modifiable:

   // ###################################################################################
   // onePatternPerItem
   // sum(p, lambda_p * a_i^p) = 1 for all i
   // is equal to:
   // 1 <= sum(p, lambda_p * a_i^p) <= 1 for all i
   // a_i^p is the coefficient in the pattern p in row i, i.e. for item i, which we produce by solving our pricing
   // subproblem

   _cons_onePatternPerItem.resize(_ins->_nbItems);

   for (int i = 0; i < _ins->_nbItems; ++i)
   {
      SCIPsnprintf(var_cons_name, 255, "onePatternPerItem_%d", i);

      SCIPcreateConsLinear(scipRMP_,                    // scip
                           &_cons_onePatternPerItem[i], // cons
                           var_cons_name,               // name
                           0,                           // nvar
                           0,                           // vars
                           0,                           // coeffs
                           1,                           // lhs
                           1,                           // rhs
                           TRUE,                        // initial
                           FALSE,                       // separate
                           TRUE,                        // enforce
                           TRUE,                        // check
                           TRUE,                        // propagate
                           FALSE,                       // local
                           TRUE,                        // modifiable
                           FALSE,                       // dynamic
                           FALSE,                       // removable
                           FALSE);                      // stick at nodes

      // our term is empty, cause we have no Lambdas
      SCIPaddCons(scipRMP_, _cons_onePatternPerItem[i]);
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

   // release variables
   for (int p = 0; p < _var_lambda.size(); p++)
   {
      SCIPreleaseVar(scipRMP_, &_var_lambda[p]);
   }

   SCIPfree(&scipRMP_);
}

// solve the problem
void Master::solve()
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
void Master::displaySolution()
{
   SCIPprintBestSol(scipRMP_, NULL, FALSE);

   return; // TODO: fix this bug here
   // display selected patterns
   cout << "Selected patterns: " << endl;
   for (auto pattern_ptr : _Patterns)
   {
      auto lambda_value = SCIPgetVarSol(scipRMP_, _var_lambda[pattern_ptr->LambdaPatternIndex]);
      if (lambda_value > 0.5)
      { // lambda value is positive, thus included
         pattern_ptr->display();
         // count total weight of included items in pattern
         int total_weight = 0;
         for (auto item : pattern_ptr->includedItems)
         {
            total_weight += _ins->par_w[item];
         }

         assert(total_weight <= _ins->par_b);

         cout << "Total weight: " << total_weight << endl
              << endl;
      }
   }
}

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
void Master::setSCIPParameters()
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
