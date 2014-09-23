/*
 * LinearScatter.h
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#ifndef LINEARSCATTER_H_
#define LINEARSCATTER_H_

#include "ModuloFlow.h"

//! Combine a linear sequence of id's in groups of a given factor
class ModuloScatter : public ModuloFlow
{
public:

  //! Default constructor
  ModuloScatter(uint32_t sink_count, uint32_t factor);

  //! Destructor
  ~ModuloScatter() {}

  //! For a given sink return all the sources
  std::vector<uint32_t> sources(uint32_t sink) const {return parent(sink);}

  //! For a given source return all the sinks
  std::vector<uint32_t> sinks(uint32_t source) const {return children(source);}

  //! For a given element return its level in the flow
  /*! For a give element compute its level which is defined as the
   *  maximal distance from a pure sink meaning that all level
   *  l elements should be processed before any level l+1 element
   *  to guarantee no deadlocks
   *  @param element: The element for which to compute the level
   *  @param return: The level of element
   */
  uint32_t level(uint32_t element) const {return mLevelCount.size() - treeLevel(element) - 1;}

  void scatterSinks(uint32_t source);

  std::vector<uint32_t> getScatterSinks() {return mScatterSink; }

  void resetScatterSinks() { mScatterSink.clear(); }
  std::vector<uint32_t> mScatterSink;

};

#endif /* LINEARSCATTER_H_ */
