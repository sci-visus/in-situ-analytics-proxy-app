#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include "PointIndex.h"
#include "Box.h"
#include "Patch.h"
#include "ParallelControl.h"
#include "ParallelFlow.h"
#include "ModuloGather.h"
#include "ModuloScatter.h"
#include "SpatialFlow.h"
#include "LocalComputeAlgorithm.h"
#include <ctime>

using namespace std;

int main(int argc, char** argv) {

  if (argc < 6) {
    printf( "Usage: %s process_id numProcX numProcY numProcZ MergeFactor \
             [min] [max] \n", argv[0]);
    return 0;
  }
  
  int rank = atoi(argv[1]);
  int num_proc_x = atoi(argv[2]);
  int num_proc_y = atoi(argv[3]);
  int num_proc_z = atoi(argv[4]);
  int merge_factor = atoi(argv[5]);
  FunctionType min_threshold = -10e34;
  FunctionType max_threshold = 10e34;

  std::cout << "Performing Join Routine for process Id: " << rank
    << " for " << num_proc_x << "x" << num_proc_y << "x" << num_proc_z
    << " processes with a merge factor of " << merge_factor << "...\n";
  if (argc > 6)
    min_threshold = atof(argv[6]);

  if (argc > 7)
    max_threshold = atof(argv[7]);

  // For modulo flow use the following
  //ModuloGather pFlow(num_proc_x*num_proc_y*num_proc_z, merge_factor);
  //ModuloScatter sFlow(num_proc_x*num_proc_y*num_proc_z, merge_factor);
  
  SpatialFlow pFlow(num_proc_x, num_proc_y, num_proc_z,
                    merge_factor, false);
  SpatialFlow sFlow(num_proc_x, num_proc_y, num_proc_z,
                    merge_factor, true );

  ParallelControl* pControl = new ParallelControl();
  
  pControl->gatherFlow(&pFlow);
  pControl->scatterFlow(&sFlow);

  pControl->SetMPIrank(rank);
  //pControl->gatherAlgorithm(GATHER_STREAMING_SORT);
  pControl->gatherAlgorithm(GATHER_PARALLEL_SORT);
  pControl->scatterAlgorithm(SCATTER_SORTED_FIX_LOCAL);
  pControl->localAlgorithm(LOCAL_DENSE_MERGE_TREE_BUILDER);
  //pControl->SetOutputFileName(outputfile);
  //pControl->localAlgorithm(LOCAL_SORTED_UF);

  //pControl->initialize(patchPerRank);
  //pControl->initialize(patches);
  pControl->initialize();
  //pControl->localCompute(min_threshold, max_threshold);
  pControl->joinTrees();
  
  // Clean Up
  delete pControl;

  return 0;
}
;

