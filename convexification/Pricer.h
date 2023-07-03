#pragma once

//================================================================
// class MyPricer: implements the functions/methods required to find
// improving variables and add them to the master problem

#include <algorithm> // for the max()/min() function

// scip includes
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

#include "../Instance.h"
#include "Master.h"

#include "SubProblem.h"
#include "SubProblemInitialColumns.h"

#include "ProductionLineSchedule.h"

using namespace std;
using namespace scip;

/**
 * @brief a class to store all informations for the pricer to solve the BPP with column generation
 *
 * @note  a class called MyPricer which is a derived class from another class called ObjPricer.
 *
 */
class MyPricer : public ObjPricer
{
public:
   shared_ptr<Instance> instance_;     // pointer to the instance
   shared_ptr<Master> master_problem_; // pointer to the master problem
   SCIP *scipRMP_;                     // pointer to the scip-env of the master-problem

   map<ProductionLine, SubProblem> subproblems_;
   map<ProductionLine, SubProblemInitialColumns> subproblems_initial_columns_;

   const char *pricer_name_;
   const char *pricer_desc_;

   // constructor
   MyPricer(shared_ptr<Master> master_problem, const char *pricer_name, const char *pricer_description, int pricer_priority, SCIP_Bool pricer_delay);

   virtual ~MyPricer();

   // to get the pointers to the variables and constraints of the transformed problem
   virtual SCIP_RETCODE scip_init(SCIP *scip, SCIP_PRICER *pricer) override;

   // to find an improving variable and to add it to the master problem
   virtual SCIP_RETCODE scip_redcost(SCIP *scip,
                                     SCIP_PRICER *pricer,
                                     SCIP_Real *lowerbound,
                                     SCIP_Bool *stopearly,
                                     SCIP_RESULT *result) override;

   // to find a variable to achieve feasibility and to add it to the master problem
   virtual SCIP_RETCODE scip_farkas(SCIP *scip, SCIP_PRICER *pricer, SCIP_RESULT *result) override;

   // perform pricing for dual and farkas combined with flag isFarkas
   SCIP_RESULT Pricing(const bool is_farkas);

private:

   void PrintMasterBounds(bool is_farkas);

   // to add the new column, i.e., the stable set, to the master problem
   void AddNewVar(shared_ptr<ProductionLineSchedule> column);

   void DisplaySchedule(shared_ptr<ProductionLineSchedule> column);

   shared_ptr<DualValues> dual_values_; // Pointer to the values of the dual variables for the current iteration of the ColumnGeneration

   bool reverse_subproblem_order_ = false;
   int redcost_iteration_ = 0;
   int farkas_iteration_ = 0;
   void foo();
   SCIP_RESULT SolveSubProblem(ProductionLine line, SubProblem& subproblem, bool is_farkas, vector<shared_ptr<ProductionLineSchedule>> solutions);
};
