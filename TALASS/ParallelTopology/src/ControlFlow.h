/*
 * ControlFlow.h
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#ifndef CONTROLFLOW_H_
#define CONTROLFLOW_H_

#include <vector>
#include "TalassConfig.h"

//! Baseclass for all control flow descriptions
class ControlFlow {
public:

  //! Default constructor
  ControlFlow() {}

  //! Destructor
  virtual ~ControlFlow() {}

  //! For a given sink return all the sources
  virtual std::vector<uint32_t> sources(uint32_t sink) const = 0;

  //! For a given source return all the sinks
  virtual std::vector<uint32_t> sinks(uint32_t source) const = 0;

  //! For a given element return its level in the flow
  /*! For a give element compute its level which is defined as the
   *  maximal distance from a pure source meaning that all level
   *  l elements should be processed before any level l+1 element
   *  to guarantee no deadlocks
   *  @param element: The element for which to compute the level
   *  @param return: The level of element
   */
  virtual uint32_t level(uint32_t id) const = 0;

  virtual uint32_t baseCount() const = 0;  

  virtual uint32_t factor() const = 0;
  
  virtual uint32_t numLevels() const = 0;

  //! Return the overal number of element
  virtual uint32_t size() const = 0;

  virtual uint32_t getBlockId(uint32_t tree_id) = 0;
};


#endif /* CONTROLFLOW_H_ */
