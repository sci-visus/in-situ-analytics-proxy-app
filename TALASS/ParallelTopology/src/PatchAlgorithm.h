/*
 * PatchAlgorithm.h
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#ifndef PATCHALGORITHM_H_
#define PATCHALGORITHM_H_

#include "DistributedDefinitions.h"
#include "Algorithm.h"
#include "Patch.h"

//! The base class for the algorithms implementing the different comparisons needed
class PatchAlgorithm : public Algorithm
{
public:

  //! Default constructor
  PatchAlgorithm(bool invert);

  ////! Local index comparison
  //bool smaller(LocalIndexType i, LocalIndexType j) const;

  ////! Local index comparison
  //bool greater(LocalIndexType i, LocalIndexType j) const;

protected:

  //! The main function with which to compare
  FunctionType* mField;
};

////! Local index comparison
//inline bool PatchAlgorithm::smaller(LocalIndexType i, LocalIndexType j) const
//{
//  if (this->mInvert)
//    return (mField[i] == mField[j]) ? (i > j) : (mField[i] > mField[j]);
//  else
//    return (mField[i] == mField[j]) ? (i < j) : (mField[i] < mField[j]);
//}
//
////! Local index comparison
//inline bool PatchAlgorithm::greater(LocalIndexType i, LocalIndexType j) const
//{
//  if (this->mInvert)
//    return (mField[i] == mField[j]) ? (i < j) : (mField[i] < mField[j]);
//  else
//    return (mField[i] == mField[j]) ? (i > j) : (mField[i] > mField[j]);
//}


#endif /* PATCHALGORITHM_H_ */
