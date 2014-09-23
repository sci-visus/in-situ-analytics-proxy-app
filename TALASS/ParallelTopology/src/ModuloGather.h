/*
 * LinearGather.h
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#ifndef LINEARGATHER_H_
#define LINEARGATHER_H_

#include "ModuloFlow.h"

//! Combine a linear sequence of id's in groups of a given factor
class ModuloGather : public ModuloFlow
{
public:

  //! Default constructor
  ModuloGather(uint32_t source_count, uint32_t factor);

  //! Destructor
  ~ModuloGather() {}

  //! For a given sink return all the sources
  std::vector<uint32_t> sources(uint32_t sink) const {return children(sink);}

  //! For a given source return all the sinks
  std::vector<uint32_t> sinks(uint32_t source) const {return parent(source);}

  //! For a given element return its level in the flow
  /*! For a give element compute its level which is defined as the
   *  maximal distance from a pure source meaning that all level
   *  l elements should be processed before any level l+1 element
   *  to guarantee no deadlocks
   *  @param element: The element for which to compute the level
   *  @param return: The level of element
   */
  uint32_t level(uint32_t element) const {return treeLevel(element);}
};




#endif /* LINEARGATHER_H_ */
