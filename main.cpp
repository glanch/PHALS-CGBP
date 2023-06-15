#include "Instance.h"
#include "CompactModel.h"
int main()
{
   auto instance = make_shared<Instance>();
   instance->read("../data/Ins_8.cal");

   auto compact_model = make_unique<CompactModel>(instance);

   compact_model->Solve();

   compact_model->DisplaySolution();
}