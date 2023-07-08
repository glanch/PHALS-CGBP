#include "Instance.h"
#include "compact/CompactModel.h"
#include "convexification/Master.h"
#include "convexification/Pricer.h"
int main(int argc, char *argv[])
{

    auto default_instance = "../data/Ins_8.cal";

    auto instance = make_shared<Instance>();

    instance->read(argc >= 2 ? argv[1] : default_instance);

    auto compact_model = make_unique<CompactModel>(instance);

    // compact_model->Solve();
    // compact_model->DisplaySolution();

    auto master_problem = make_shared<Master>(instance);

    MyPricer *pricer = new MyPricer(
        master_problem,
        "PHALS_exact_mip",                                                      // name of the pricer
        "PHALS Pricer with convexification and braching on original variables", // short description of the pricer
        0,                                                                      //
        TRUE);                                                                  //

    SCIPincludeObjPricer(master_problem->scipRMP_, //
                         pricer,
                         true);

    // activate pricer_VRP_exact_mip
    SCIPactivatePricer(master_problem->scipRMP_, SCIPfindPricer(master_problem->scipRMP_, pricer->pricer_name_));

    master_problem->Solve();
    master_problem->DisplaySolution();
}