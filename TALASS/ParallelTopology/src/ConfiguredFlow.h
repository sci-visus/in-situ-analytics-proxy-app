/*
 * ConfiguredFlow.h
 *
 *  Created on: Sept 27, 2012
 *      Author: jediati
 */

#ifndef CONFIGUREDFLOW_H_
#define CONFIGUREDFLOW_H_

#define ATI_DEBUG 1


#ifndef ATI_DEBUG
#include "ControlFlow.h"
#else 
#include <vector>
#endif








//! Combine a linear sequence of id's in groups of a given factor
#ifndef ATI_DEBUG
class ConfiguredFlow : public ControlFlow
#else
class ConfiguredFlow
#endif
{

protected:
	struct point3 {
		point3(int xx, int yy, int zz) : x(xx), y(yy), z(zz) {}
		int x;
		int y;
		int z;
	};

   std::vector<point3> mLevelDims;
   std::vector<point3> mLevelMerge;

	uint32_t mTotalBlocks;
	// the number of blocks in each level
   std::vector<int> mLevelSizes;
	int mNumLevels;

public:

  //! Default constructor
  ConfiguredFlow(char* config_file) {
	  
	  int x_blocks, y_blocks, z_blocks, x_m, y_m, z_m;
	  FILE* fin = fopen(config_file, "r");
	  fscanf(fin, "%d\n", &mNumLevels);
	  fscanf(fin, "%d %d %d\n", &x_blocks, &y_blocks, &z_blocks);	  
	  mLevelDims.push_back( point3(x_blocks, y_blocks, z_blocks));
	  
  	  mTotalBlocks = 0;
	  mLevelSizes.push_back(mTotalBlocks);

	  for (int mcl = 0; mcl < mNumLevels; mcl++) {
		  mTotalBlocks +=x_blocks*y_blocks*z_blocks;
		  mLevelSizes.push_back(mTotalBlocks);
		  fscanf(fin, "%d %d %d\n", &x_m, &y_m, &z_m);
		  mLevelMerge.push_back( point3(x_m, y_m, z_m));
	      int xx, yy, zz;
		  xx = x_blocks / x_m;
			if (x_blocks % x_m > 0) xx++;
				yy = y_blocks / y_m;
				if (y_blocks % y_m > 0) yy++;
				zz = z_blocks / z_m;
				if (z_blocks % z_m > 0) zz++;
	 	  mLevelDims.push_back( point3(xx, yy, zz));

		  x_blocks = xx;
		  y_blocks = yy;
		  z_blocks = zz;

	  }

	  mTotalBlocks += x_blocks*y_blocks*z_blocks;
     mLevelSizes.push_back(mTotalBlocks);
     fclose(fin);

	  for (int i =0; i < mNumLevels; i++) {
		  printf("%d->%d<-->(%d, %d, %d)<-->(%d, %d, %d)\n", i, mLevelSizes[i],
			  mLevelDims[i].x, mLevelDims[i].y, mLevelDims[i].z,
           mLevelMerge[i].x, mLevelMerge[i].y, mLevelMerge[i].z);
	  }
   for (int i =0; i < mLevelSizes.size(); i++) {
      printf("level-%d is %d\n", i, mLevelSizes[i]);
   }
  }

  //! Destructor
  ~ConfiguredFlow() {}

  //! For a given sink return all the sources
  std::vector<uint32_t> sources(uint32_t sink) const {
     std::vector<uint32_t> res;
	  int tlev = 0;
	  while ( mLevelSizes[tlev+1] <= sink) tlev++;

	  //printf("found sink-%d in level %d-%d\n", sink, tlev, mLevelSizes[tlev]);

     if (tlev == 0) return res;

	  // get which block of current level
	  int cblock = sink - mLevelSizes[tlev];

	  //push the previous level's blocks to the result
	  
	  // coordinates in level tlev
	  point3 coords(
		  cblock % mLevelDims[tlev].x,
		  (cblock / mLevelDims[tlev].x) % mLevelDims[tlev].y,
		  cblock / (mLevelDims[tlev].x*mLevelDims[tlev].y)
		  );
	  // base coordinates in level tlev-1
	  point3 ncoords(
		  coords.x*mLevelMerge[tlev-1].x,
		  coords.y*mLevelMerge[tlev-1].y,
		  coords.z*mLevelMerge[tlev-1].z
		  );
     //printf("old(%d, %d, %d) new (%d, %d, %d)\n",
     //      coords.x, coords.y, coords.z,
     //      ncoords.x, ncoords.y, ncoords.z);

   int iext = mLevelMerge[tlev - 1].x;
   int jext = mLevelMerge[tlev - 1].y;
   int kext = mLevelMerge[tlev - 1].x;


			  for (int k = 0; k < kext; k++) {
		  for (int  j =0; j < jext; j++){
	  for (int i = 0; i < iext; i++) {
				  // the coordinates in the current level of the source block
				  point3 nncoords(ncoords.x + i, ncoords.y + j, ncoords.z + k);
  
              //printf("nnc(%d, %d, %d), ld(%d, %d, %d)\n",
              //      nncoords.x, nncoords.y, nncoords.z, 
              //      mLevelDims[tlev-1].x, mLevelDims[tlev-1].y, mLevelDims[tlev-1].z);

              if (nncoords.x > mLevelDims[tlev-1].x - 1) continue;
              if (nncoords.y > mLevelDims[tlev-1].y - 1) continue;
              if (nncoords.z > mLevelDims[tlev-1].z - 1) continue;
				  //printf("XX\n");
              int tres = nncoords.x + 
					  nncoords.y * mLevelDims[tlev-1].x +
					  nncoords.z * mLevelDims[tlev-1].x * mLevelDims[tlev-1].y;
				  tres += mLevelSizes[tlev-1];
				  res.push_back(tres);
			  }
		  }
	  }

	  return res;
  
  }

  //! For a given source return all the sinks
  std::vector<uint32_t> sinks(uint32_t source) const {
     std::vector<uint32_t> res;
	  
     
	  int tlev = 0;
	  while ( mLevelSizes[tlev+1] <= source ) tlev++;

	  //printf("found source-%d in level %d-%d\n", source, tlev, mLevelSizes[tlev]);

	  if (tlev == mNumLevels) return res;

	  // get which block of current level
	  int cblock = source - mLevelSizes[tlev];
	  
	  //push the previous level's blocks to the result
	  
	  // coordinates in level tlev
	  point3 coords(
		  cblock % mLevelDims[tlev].x,
		  (cblock / mLevelDims[tlev].x) % mLevelDims[tlev].y,
		  cblock / (mLevelDims[tlev].x*mLevelDims[tlev].y)
		  );
	  // base coordinates in level tlev+1
	  point3 ncoords(
		  coords.x/mLevelMerge[tlev].x,
		  coords.y/mLevelMerge[tlev].y,
		  coords.z/mLevelMerge[tlev].z
		  );
     int tres = ncoords.x +
        ncoords.y * mLevelDims[tlev+1].x +
        ncoords.z * mLevelDims[tlev+1].x * mLevelDims[tlev+1].y;
     tres += mLevelSizes[tlev+1];

     
     res.push_back(tres);
     
     return res;
  }

  //! For a given element return its level in the flow
  /*! For a give element compute its level which is defined as the
   *  maximal distance from a pure source meaning that all level
   *  l elements should be processed before any level l+1 element
   *  to guarantee no deadlocks
   *  @param element: The element for which to compute the level
   *  @param return: The level of element
   */
  uint32_t level(uint32_t element) const {
     int tlev = 0;
     //printf("elemend->%d, mLevelSizes[0]->%d\n",element, mLevelSizes[0]);
     
	  while ( mLevelSizes[tlev+1] <= element ) {
        tlev++;
      //printf("elemend->%d, mLevelSizes[%d]->%d\n",element, tlev,mLevelSizes[tlev]);
     }
     return tlev;
  }

  uint32_t totalCount() {
     return mTotalBlocks;
  }

};




#endif /* LINEARGATHER_H_ */
