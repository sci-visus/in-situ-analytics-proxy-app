/*
 * PointIndex.h
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#ifndef POINTINDEX_H_
#define POINTINDEX_H_

#include "DistributedDefinitions.h"
#include "TalassConfig.h"

//! The typedef to define the grid index space
typedef int GridIndex;

//! A PointIndex uniquely indexes a point in the domain
class PointIndex
{
public:

  //! Default constructor
  PointIndex(GridIndex i=0, GridIndex j=0, GridIndex k=0) { mIndex[0] = i, mIndex[1] = j; mIndex[2] = k; }

  //! Copy constructor
  PointIndex(const PointIndex& p) { for(int i=0; i < 3; i++) mIndex[i] = p.mIndex[i]; }

  //! Destructor
  ~PointIndex() {}

  //! Access operator
  GridIndex operator[](int i) const {return mIndex[i];}

  GridIndex& operator[](int i) {return mIndex[i];}

  //! Index conversion to the flat local index space
  /*! Convert this PointIndex into the flat index space (row-major)
   *  of the local box assuming its overall dimensions are as given
   * @param dim: The size of the local box
   * @return The corresponding row-major index
   */
  int local(const PointIndex& dim) const;

  //! Index conversion to the flat global index space
  /*! Convert this PointIndex into the float index space (row-major)
   *  of the global index space assuming the local index space is
   *  described by the box whose lower left corner is low and the 
   *  global box with dimension dim.
   * @param low: PointIndex of the left most corner of the local box
   *             in global index space
   * @param dim: Dimensions of the global index space
   * @return The corresponding row-major index
   */
  int global(const PointIndex& low, const PointIndex& dim) const;

  //! Returns size of block associated with indices 
  int size() const { return mIndex[0]*mIndex[1]*mIndex[2]; }

  //! return whether or not this pointIndex is greater than zero and less than the dimensions provided 
  bool inRange(const PointIndex &dim) const { 
    for(int i=0; i < 3; i++) {
      if(mIndex[i] < 0) return false;
      if(mIndex[i] >= dim[i]) return false;
    }
    return true;
  }

private:

  //! The array of indices
  GridIndex mIndex[3];
};


#endif /* POINTINDEX_H_ */
