/*
 * Segmentation.h
 *
 *  Created on: Jan 30, 2012
 *      Author: bremer5
 */

#ifndef SEGMENTATION_H_
#define SEGMENTATION_H_

#include <vector>
#include "Patch.h"

//! Maintains a list of indices for a list of segments
class Segmentation
{
public:

  //! Internal typedef in case we want to implement our own array
  typedef std::vector<LocalIndexType> SegmentType;

  //! internal typedef in case we want to switch mapping types (e.g. use hash)
  typedef std::map<GlobalindexType,SegmentType> MapType;

  //! Constructor
  Segmentation(const Patch<FunctionType>& patch);

  //! Destructor
  ~Segmentation() {}


  //! Create a new segment with the given id. Add the id to the segment
  int addSegment(GlobalIndexType seg_id, LocalIndexType id);

  //! Create a new segment with the given id from the array
  int addSegment(GlobalIndexType seg_id, const SegmentType& seg);

  //! Add the given id to the given segment
  int addSample(GlobalIndexType seg_id, LocalIndexType id);

  //! Return a reference to a segment
  SegmentType& segment(GlobalIndexType seg_id);

private:

  //! The map from ids to segments
  MapType mSegments;

  //! The reference to the original patch
  const Patch<FunctionType>& mPatch;
};


#endif /* SEGMENTATION_H_ */
