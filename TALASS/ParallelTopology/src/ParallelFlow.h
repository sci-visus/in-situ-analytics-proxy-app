/*
 * ParallelFlow.h
 *
 *  Created on: July 16, 2012
 *      Author: landge1
 */

#ifndef PARALLELFLOW_H_
#define PARALLELFLOW_H_

#include <vector>
#include "ControlFlow.h"

//! A partial specialization of a ControlFlow based on modulo operations
class ParallelFlow : public ControlFlow
{
public:

  //! Default constructor
  ParallelFlow(uint32_t base_count, uint32_t factor, uint32_t rank);

  //! Destructor
  virtual ~ParallelFlow() {}

  virtual std::vector<uint32_t> sources(uint32_t sink) const {
	  std::vector<uint32_t> source;
	  source = children(sink);
	  return source;
  }

  virtual std::vector<uint32_t> sinks(uint32_t source) const {
	  std::vector<uint32_t> sinks;
	  sinks = parent(source);
	  return sinks;
  }

  virtual uint32_t level(uint32_t id) const {return 0;}
  //! Return the overal number of element
  virtual uint32_t size() const {return 1; }

  uint32_t factor() const {
	  return mFactor;
  };

protected:

  //! The modulo factor used
  uint32_t mFactor;
  
  uint32_t mNumLeafElements;

  uint32_t mRank;

  //! The number of elements per level
  //std::vector<uint32_t> mLevelCount;

  //! The aggregate count of elements of previous levels
  //td::vector<uint32_t> mAggregatedCount;

  //! Return the children of the tree
  std::vector<uint32_t> children(uint32_t id) const;

  //! Return the parent
  std::vector<uint32_t> parent(uint32_t id) const;

  //! Function to return the internal level
  uint32_t treeLevel(uint32_t id) const;


};


#endif /* PARALLELFLOW_H_ */
