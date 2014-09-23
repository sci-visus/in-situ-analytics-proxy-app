/*
 * Algorithm.h
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#ifndef ALGORITHM_H_
#define ALGORITHM_H_

#include "DistributedDefinitions.h"
#include "MergeTree.h"

//! The base class for the algorithms implementing the different comparisons needed
class Algorithm
{
public:

  //! Default constructor
  Algorithm(bool invert);

  //! Local node comparison according to the flag
  bool smaller(const TreeNode& n0, const TreeNode& n1) const;

  //! Local node comparison according to the flag
  bool greater(const TreeNode& n0, const TreeNode& n1) const;

protected:

  //! The internal flag determining which comparison we use
  const bool mInvert;
};


inline bool Algorithm::smaller(const TreeNode& n0, const TreeNode& n1) const
{
  if (mInvert)
    return (n0.value() == n1.value()) ? (n0.id() > n1.id()) : (n0.value() > n1.value());
  else
    return (n0.value() == n1.value()) ? (n0.id() < n1.id()) : (n0.value() < n1.value());
}

//! Local node comparison according to the flag
inline bool Algorithm::greater(const TreeNode& n0, const TreeNode& n1) const
{
  if (mInvert)
    return (n0.value() == n1.value()) ? (n0.id() < n1.id()) : (n0.value() < n1.value());
  else
    return (n0.value() == n1.value()) ? (n0.id() > n1.id()) : (n0.value() > n1.value());
}


#endif /* ALGORITHM_H_ */
