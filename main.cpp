#include "Instance.h"
#include "compact/CompactModel.h"
#include "convexification/Master.h"
#include "convexification/Pricer.h"
int main(int argc, char *argv[])
{
    auto default_instance = "../data/Ins_8.cal";

    // if a parameter is passed, this is used as file path, else default_instance is used
    auto instance_path = argc >= 2 ? argv[1] : default_instance; 
    
    auto instance = make_shared<Instance>();

    // read instance
    instance->read(instance_path);

    // if a parameter is passed, this is used as time limit in seconds, else default time limit is used
    auto time_limit = argc >= 3 ? stod(argv[2]) : Settings::kDefaultTimeLimit;

    // create compact model
    auto compact_model = make_unique<CompactModel>(instance);

    // solve and display solution
    compact_model->Solve(time_limit);
    compact_model->DisplaySolution();
    
    // create master problem
    auto master_problem = make_shared<Master>(instance);

    // create pricer for linking SubProblem and Master in B&P algo
    MyPricer *pricer = new MyPricer(
        master_problem,
        "PHALS_exact_mip",                                                      // name of the pricer
        "PHALS Pricer with convexification and braching on original variables", // short description of the pricer
        0,                                                                      //
        TRUE);                                                                  //

    // include pricer in Master SCIP object
    SCIPincludeObjPricer(master_problem->scipRMP_, //
                         pricer,
                         true);

    // activate pricer
    SCIPactivatePricer(master_problem->scipRMP_, SCIPfindPricer(master_problem->scipRMP_, pricer->pricer_name_));

    
    master_problem->Solve(time_limit);
    master_problem->DisplaySolution();
}