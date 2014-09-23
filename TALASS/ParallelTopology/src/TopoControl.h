/*
 * TopoControl.h
 *
 *  Created on: Jan 30, 2012
 *      Author: bremer5
 */

#ifndef TOPOCONTROL_H_
#define TOPOCONTROL_H_

#include <vector>
#include "Patch.h"

//! The baseclass to compute parallel topology
class TopoControl
{
public:

  //! Default constructor
  TopoControl() {}

  //! Destructor
  virtual ~TopoControl() {}

  //! Initialize the computation with the given patches
  virtual int initialize(const std::vector<Patch*> &patches, uint8_t block_bits) = 0;

  //! Compute the tree, segmentation, and statistics
  virtual int compute(FunctionType low_threshold=-10e34, FunctionType high_threshold=10e34) = 0;

  //! Dump the results
  virtual int save() = 0;
};


#endif /* TOPOCONTROL_H_ */
