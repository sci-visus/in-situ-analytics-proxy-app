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

  if (argc < 5) {
    printf( "Usage: %s inputfile gXDim gYDim gZDim [min] [max] \n", argv[0]);
    return 0;
  }
  
  FunctionType min_threshold = -10e34;
  FunctionType max_threshold = 10e34;

  if (argc > 5)
    min_threshold = atof(argv[10]);

  if (argc > 6)
    max_threshold = atof(argv[11]);

  int data_size[3];
  data_size[0] = atoi(argv[2]);
  data_size[1] = atoi(argv[3]);
  data_size[2] = atoi(argv[4]);

  FunctionType *dataPtr = (FunctionType*)malloc(sizeof(FunctionType)*
                                      data_size[0]*data_size[1]*data_size[2]);

  FILE *fp = fopen(argv[1], "rb");
  fread((void*)dataPtr, sizeof(FunctionType), 
        data_size[0]*data_size[1]*data_size[2], fp);
  fclose(fp);

  PointIndex origin(0, 0, 0);
  PointIndex global_dimensions(data_size[0], data_size[1], data_size[2]);
  Box global(origin,global_dimensions,origin,global_dimensions);
  //PointIndex num_buckets(xDim, yDim, zDim);

  vector<Patch*> patches;
  PointIndex low, high;

  std::vector<FunctionType*> data(1,NULL);
  data[0] = dataPtr;

  low = PointIndex(0, 0, 0);
  high = PointIndex(data_size[0]-1, data_size[1]-1, data_size[2]-1);
  //fprintf(stderr,"Box [%d,%d,%d] x [%d,%d,%d] Values::%f\n",low[0],low[1],low[2],high[0],high[1],high[2],data[0][6]);

  SubBox box(&global, low, high, low, high);
  //SubBox box(&global, low, high, origin, global_dimensions);
  
  Patch* p = new Patch(data,box);
  patches.push_back(p);

  // For modulo flow use the following
  ModuloGather pFlow(2,2);
  ModuloScatter sFlow(2,2);
  
  ParallelControl* pControl = new ParallelControl();
  
  pControl->gatherFlow(&pFlow);
  pControl->scatterFlow(&sFlow);

  pControl->SetMPIrank(0);
  //pControl->gatherAlgorithm(GATHER_STREAMING_SORT);
  pControl->gatherAlgorithm(GATHER_PARALLEL_SORT);
  pControl->scatterAlgorithm(SCATTER_SORTED_FIX_LOCAL);
  pControl->localAlgorithm(LOCAL_DENSE_MERGE_TREE_BUILDER);
  //pControl->SetOutputFileName(outputfile);
  //pControl->localAlgorithm(LOCAL_SORTED_UF);

  //pControl->initialize(patchPerRank);
  pControl->initialize(patches);
  pControl->localCompute(min_threshold, max_threshold);
  
  // Clean Up
  // TODO: Some data related clean up needs to be done, but shall do that once we have
  // the functionality to read only desired patches
  delete pControl;

  return 0;
}
;

