/*
 * ModuloFlow.h
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#ifndef MODULOFLOW_H_
#define MODULOFLOW_H_

#include <vector>
#include "ControlFlow.h"

//! A partial specialization of a ControlFlow based on modulo operations
class ModuloFlow : public ControlFlow
{
public:

  //! Default constructor
  ModuloFlow(uint32_t base_count, uint32_t factor);

  //! Destructor
  ~ModuloFlow() {}

  //! Return the number of elements in the first level
  uint32_t baseCount() const {return mBaseCount;}
  
  //! Return the number of levels in the tree
  uint32_t numLevels() const {return mAggregatedCount.size()-1;}

  //! Return the overal number of element
  uint32_t size() const {return mAggregatedCount.back();}

  uint32_t factor() const {return mFactor;};

  uint32_t getBlockId(uint32_t tree_id) {return 1;}

protected:

  //! The modulo factor used
  uint32_t mFactor;

  uint32_t mBaseCount;

  //! The number of elements per level
  std::vector<uint32_t> mLevelCount;

  //! The aggregate count of elements of previous levels
  std::vector<uint32_t> mAggregatedCount;

  //! Return the children of the tree
  std::vector<uint32_t> children(uint32_t id) const;

  //! Return the parent
  std::vector<uint32_t> parent(uint32_t id) const;

  //! Function to return the internal level
  uint32_t treeLevel(uint32_t id) const;
};


#endif /* MODULOFLOW_H_ */
