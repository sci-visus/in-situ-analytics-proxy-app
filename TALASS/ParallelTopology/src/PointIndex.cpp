#include "PointIndex.h"

int PointIndex::local(const PointIndex& dim) const {
  int index=0, chunkSize=1;

  for(int i=0; i < 3; i++) {
    index += mIndex[i]*chunkSize;  
    chunkSize *= dim[i];
  }

  return index;

}

int PointIndex::global(const PointIndex& low, const PointIndex& dim) const {

  PointIndex globalPointIndex;
  
  // get the global pointIndex
  for(int i=0; i < 3; i++) {
    globalPointIndex[i] = low[i] + mIndex[i];   
  }

  // return its local flat index with respect to the global dimensions
  return globalPointIndex.local(dim);
}


