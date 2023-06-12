#include "CompactModel.h"
#include <scip/scip_general.h>
#include <scip/scip_prob.h>
#include "Settings.h"

/**
 * @brief Construct a new Compact Model:: Compact Model object
 *
 * @param ins pointer to problem-instance
 *
 * @note This code is a constructor for the CompactModel class. It creates a SCIP environment and sets the specific
 * parameters. It then creates and adds all variables to the model, including binary variables X_ij and Y_i for items i,
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
   setSCIPParameters();

   // create helping-dummy for the name of variables and constraints
   char var_cons_name[Settings::kSCIPMaxStringLength];

   // #####################################################################################################################
   //  Create and add all variables
   // #####################################################################################################################

   for(auto& line : instance_->productionLines) {
      for(auto& coil_i : instance_->coils) {
         for(auto& coil_j : instance_->coils) {
            if(i != j) {
               for(auto& mode_i : instance_->modes[make_tuple<coil_i, line]) {
                  for(auto& mode_j : instance_->modes[make_tuple<coil_j, line]) {
                     // create var here
                  }  
               }

            }
         }  
      }
   }
   for( int j = 0; j < _ins->_nbBins; ++j )
   {
      SCIPsnprintf(var_cons_name, Settings::kSCIPMaxStringLength, "Y_%d", j); // set name for debugging

      SCIPcreateVarBasic(scip_,              //
                         &_var_Y[j],           // returns the address of the newly created variable
                         var_cons_name,        // name
                         0,                    // lower bound
                         1,                    // upper bound
                         1,                    // objective function coefficient, this is equal to 1 according to (1)
                         SCIP_VARTYPE_BINARY); // variable type

      SCIPaddVar(scip_, _var_Y[j]); // add var to scip-env
   }

   // #####################################################################################################################
   //  binary variable X_ij

   // set all dimensions for X_ij, with empty pointers
   _var_X.resize(_ins->_nbItems); // first dimension of X_ij is equal to the amount of items in this instance

   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      _var_X[i].resize(_ins->_nbItems,
                       nullptr); // second dimension of X_ij is equal to the amount of bins in this instance
   }
   // create and add the variable Y_ij to the model
   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      for( int j = 0; j < _ins->_nbBins; ++j )
      {
         SCIPsnprintf(var_cons_name, 255, "X_%d_%d", i, j); // set name

         SCIPcreateVarBasic(scip_,
                            &_var_X[i][j], // returns the address of the newly created variable
                            var_cons_name, // name
                            0,             // lower bound
                            1,             // upper bound
                            0, // objective function coefficient, this is equal to 0 because the variable does not
                               // appear in the objective (1)
                            SCIP_VARTYPE_BINARY); // variable type

         SCIPaddVar(scip_, _var_X[i][j]); // add var to scip-env
      }
   }
   // #####################################################################################################################
   //  Add restrictions
   // #####################################################################################################################

   // #####################################################################################################################
   //  restriction (2) in lecture handout: unique assignment constraints

   // sum(j in J, X_ij) = 1 for all i in I
   // is equal to:
   // 1 <= sum(j in J, X_ij) <= 1 for all i in I

   // set all dimension for constraint with empty pointer

   _cons_unique_assignment.resize(_ins->_nbItems, nullptr); // dimension is equal to the number of items in theinstance

   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      SCIPsnprintf(var_cons_name, 255, "unique_assignment_%d", i);

      SCIPcreateConsBasicLinear(scip_,                     // scip
                                &_cons_unique_assignment[i], // cons
                                var_cons_name,               // name
                                0,                           // nvar
                                0,                           // vars
                                0,                           // coeffs
                                1,                           // lhs
                                1);                          // rhs

      for( int j = 0; j < _ins->_nbBins; ++j ) // sum over all bins j in J
      {
         SCIPaddCoefLinear(scip_, _cons_unique_assignment[i], _var_X[i][j], 1);
      }

      SCIPaddCons(scip_, _cons_unique_assignment[i]);
   }

   // #####################################################################################################################
   //  restriction (3) in lecture handout: capacity and linking constraints

   // sum(i in I, w_i * X_ij) <= b * Y_j for all bins j in J
   // is equal to:
   // -infty <= sum(i in I, w_i * X_ij) - b * Y_j <= 0 for all bins j in J

   // set all dimensions
   _cons_capacity_and_linking.resize(_ins->_nbBins,
                                     nullptr); // dimension is equal to the number of bins in this instance

   for( int j = 0; j < _ins->_nbBins; ++j )
   {
      SCIPsnprintf(var_cons_name, 255, "capacity_and_linking_%i", j); // set constraint name for debugging

      SCIPcreateConsBasicLinear(scip_,                        // scip
                                &_cons_capacity_and_linking[j], // cons
                                var_cons_name,                  // name
                                0,                              // number of variables
                                0,                              // vars
                                0,                              // coeffs
                                -SCIPinfinity(scip_),         // lhs
                                0);                             // rhs

      for( int i = 0; i < _ins->_nbItems; ++i ) // sum over all items i in I
      {
         SCIPaddCoefLinear(scip_,                       // scip-env
                           _cons_capacity_and_linking[j], // constraint
                           _var_X[i][j],                  // variable
                           _ins->par_w[i]);               // coefficient
      }
      SCIPaddCoefLinear(scip_, _cons_capacity_and_linking[j], _var_Y[j], -ins->par_b);
      SCIPaddCons(scip_, _cons_capacity_and_linking[j]); // add constraint to the scip-env
   }

   // #####################################################################################################################
   //  Generate LP file
   // #####################################################################################################################

   // Generate a file to show the LP-Program, that is build. "FALSE" = we get our specific choosen names.
   SCIPwriteOrigProblem(scip_, "compact_model_bpp.lp", "lp", FALSE);
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
   // #####################################################################################################################
   //  release constraints
   // #####################################################################################################################
   //  Every constraint that we have generated and stored needs to be released. Thus, use the same for-loops as for
   //  generating the constraints to ensure that you release everyone.

   // release all unique assignment constraints
   for( int i = 0; i < _ins->_nbItems; ++i )
   {
      SCIPreleaseCons(scip_, &_cons_unique_assignment[i]);
   }

   // release all capacity and linking constraints

   for( int j = 0; j < _ins->_nbBins; j++ )
   {

      SCIPreleaseCons(scip_, &_cons_capacity_and_linking[j]);
   }

   // #####################################################################################################################
   //  release all variables
   // #####################################################################################################################

   // release all X_ij - variables

   for( int i = 0; i < _ins->_nbItems; ++i ) // sum over all i in I
   {
      for( int j = 0; j < _ins->_nbBins; ++j ) // sum over all j in J
      {
         SCIPreleaseVar(scip_, &_var_X[i][j]);
      }
   }

   // release all Y_j - variables
   for( int j = 0; j < _ins->_nbBins; ++j ) // sum over all bins j in J
   {
      SCIPreleaseVar(scip_, &_var_Y[j]);
   }

   // #####################################################################################################################
   //  release SCIP object
   // #####################################################################################################################
   //  At the end release the SCIP object itself
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
void CompactModel::solve()
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
void CompactModel::displaySolution() { SCIPprintBestSol(scip_, NULL, FALSE); };
