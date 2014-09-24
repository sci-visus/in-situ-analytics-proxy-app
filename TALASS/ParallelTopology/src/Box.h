/*
 * Box.h
 *
 *  Created on: Jan 30, 2012
 *      Author: bremer5
 */

#ifndef BOX_H_
#define BOX_H_

#include "DistributedDefinitions.h"
#include "PointIndex.h"
#include <iostream>

#define FRONT 0b00100000
#define BACK  0b00010000
#define NORTH 0b00001000
#define SOUTH 0b00000100
#define EAST  0b00000010
#define WEST  0b00000001



//! A box encapsulates the notion of a discrete region of index space
class Box
{
public:

  //! Default constructor creatign an invalid box
  Box();

  //! Construct a sub-box
  /*! Create a local sub-box covering the index interval [low,high]
   *  and assume the global array has the given dimensions
   * @param low: Left most corner of the local index space
   * @param high: Right most corner of the local index space
   * @param dim: Size of the global index space
   */
  
  
  Box(const PointIndex& low, const PointIndex& high, 
	  const PointIndex& fieldLow, const PointIndex& fieldHigh) : 
	mLow(low), mHigh(high), mFieldLow(fieldLow), mFieldHigh(fieldHigh)  {
	  for (int i = 0; i < 3; i++) {
		  mLocalDimensions[i] = mHigh[i] - mLow[i];
		  mDimensions[i] = mFieldHigh[i] - mFieldLow[i];
		  mLowOffset[i] = low[i] - fieldLow[i];
	  }
  };

  //! returns the total size of the global index space
 // GlobalIndexType globalSize() const { return mDimensions.size(); };

  LocalIndexType localSize() const {
	  return mLocalDimensions.size();
  }

  LocalIndexType localIndex(PointIndex p) const {
	  return p[0] + p[1] * mLocalDimensions[0] +
		  p[2] * mLocalDimensions[0] * mLocalDimensions[1];
  }

  PointIndex coordinates(LocalIndexType index) const {
	  //++ return the PointIndex of my localIndexType
	  // in global? or local?
	  PointIndex result(
           index % mLocalDimensions[0],
           (index / mLocalDimensions[0]) % mLocalDimensions[1],
           index / (mLocalDimensions[0]*mLocalDimensions[1]));
	  return result;
  } 


  //PointIndex globalCoordinates(GlobalIndexType index) const {

	 //PointIndex result(
  //         (index % mDimensions[0]) + this->mFieldLow[0],
  //         ((index / mDimensions[0]) % mDimensions[1]) + this->mFieldLow[1],
  //         (index / (mDimensions[0]*mDimensions[1])) + this->mFieldLow[2]);
	 // return result;

  //}

  //int globalBoundaryCount(LocalIndexType index) const {
  //   int tBC = 0; 
  //    PointIndex pL = coordinates(index);
  //    
	 // // get the coordinates in global space
	 // for (int i = 0; i < 3; i++) pL[i] += this->mLow[i];

  //    for (int i = 0; i < 3; i++) tBC += (pL[i] == 0 ? 1: 0);
  //    for (int i = 0; i < 3; i++) tBC += (pL[i] == (mDimensions[i] + mFieldLow[i]) - 1 ? 1 : 0);
  //    return tBC;


  //}

  
  LocalIndexType fieldIndex(LocalIndexType index) {
    PointIndex p = this->coordinates(index);
    for (int i = 0; i < 3; i++) p[i] += this->mLowOffset[i];
    return p[0] + 
    p[1] * this->mDimensions[0] +
    p[2] * this->mDimensions[0] * this->mDimensions[1];
  }


  	bool insideBox(PointIndex p) const {
      for (int i = 0; i < 3; i++) {
         if (p[i]  < 0 || p[i] >= localDimensions()[i]) return false;
      }
      return true;
	}



  bool subBoundary(PointIndex p, LocalIndexType nid) {
      
	  int pbdc = boundaryCount(p);
	  if (pbdc == 0) return true;
      PointIndex np = coordinates(nid);
      for (int i = 0; i < 3; i++) {
		  if (p[i] == 0 && np[i] != 0) return false;
		  if (p[i] == mLocalDimensions[i]-1 && np[i] != mLocalDimensions[i]-1) return false;
	  }
      return true;
  }

 int boundaryCount(PointIndex p) const {
      int tBC = 0; 
      for (int i = 0; i < 3; i++) {
		  if (p[i] == 0 || p[i] == mLocalDimensions[i]-1) tBC++;
	  }
      //for (int i = 0; i < 3; i++) tBC += (p[i] == mLocalDimensions[i]-1 ? 1 : 0);
      return tBC; //++ NEEDS BOUNDARY COUNT
  }
  int boundaryCount(LocalIndexType index) const {
      PointIndex p = coordinates(index);
	  return boundaryCount(p);
  }

  // Label to the boundary node will determine which boundary the node belongs
  // to. This will help us to communicate only specific boundaries to the
  // neighbors. 
  // The label is formed using the following bits: 0x00FBNSEW where
  // F: Front, B:Back, N:North, S:South, E:East, W:West
  // Based on the boundary node's location the appropriate bits are set 
  
  char boundaryLabel(PointIndex p) const {
  /*    char label = 0;
      //std::cout << "Local Dim: " << mLocalDimensions[0] 
      //          << "," << mLocalDimensions[1] 
      //          << "," << mLocalDimensions[2] << std::endl; 
      if (p[0] == 0) label = label | WEST;
      if (p[0] == mLocalDimensions[0]-1) label = label | EAST; 
      if (p[1] == 0) label = label | SOUTH;
      if (p[1] == mLocalDimensions[1]-1) label = label | NORTH; 
      if (p[2] == 0) label = label | BACK;
      if (p[2] == mLocalDimensions[2]-1) label = label | FRONT;

      return label;
  */
  }
  
  char boundaryLabel(LocalIndexType index) const {
      PointIndex p = coordinates(index);
      return boundaryLabel(p);
  }
  //! Destructor
  ~Box() {}

  const PointIndex& localDimensions() const {
     return mLocalDimensions;
  }


  PointIndex lowCorner() { return mLow; }
  PointIndex highCorner() { return mHigh; }
  //PointIndex globalDimensions() { return mDimensions; }

protected:

  //! The dimensions of the local index space
	PointIndex mLocalDimensions;

  //! The left most corner of the index box
  PointIndex mLow;

  //! The right most corner of the index box
  PointIndex mHigh;

  //! The dimensions of the global index space
  PointIndex mDimensions;

    //! The left most corner of the index box
  PointIndex mFieldLow;

  //! The right most corner of the index box
  PointIndex mFieldHigh;

  	PointIndex mLowOffset;


};


class SubBox : public Box {

public:
	SubBox(Box* parent, const PointIndex& low, const PointIndex& high, 
	  const PointIndex& fieldLow, const PointIndex& fieldHigh) :  
	  Box(low, high, fieldLow, fieldHigh), mParent(parent)
	{

	}

  //LocalIndexType fieldIndex(LocalIndexType index) {
  //  return index;
  //}


 GlobalIndexType globalIndex(PointIndex p) const {
	 for (int i = 0; i < 3; i++) p[i] += this->mLow[i];
	 return mParent->localIndex(p);
  }

  GlobalIndexType globalIndex(LocalIndexType index) const {
	 PointIndex p = this->coordinates(index);
	 return globalIndex(p);
  }

  int globalBoundaryCount(LocalIndexType index) const {
     GlobalIndexType gid = globalIndex(index);
	 return mParent->boundaryCount(gid);
  }


  PointIndex gCoordinates(LocalIndexType index) const {
	  GlobalIndexType gid = globalIndex(index);
	 return mParent->coordinates(gid);
  }


protected:


	Box* mParent;
};


#endif /* BOX_H_ */
