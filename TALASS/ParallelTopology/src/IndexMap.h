/*
 * IndexMap.h
 *
 *  Created on: Jan 30, 2012
 *      Author: bremer5
 */

#ifndef INDEXMAP_H_
#define INDEXMAP_H_

//! A class to implement a surjective map of one set of indices onto another
template <typename DomainType, typename RangeType>
class IndexMap
{
public:

  //! Default constructor
  IndexMap();

  //! Destructor
  virtual ~IndexMap() {}

  //! Map a point in the domain into the range
  virtual RangeType operator()(DomainType id) const = 0;

  //! For a point in the range return the pre-image
  virtual std::vector<DomainType> preimage(RangeType id) const = 0;
};



#endif /* INDEXMAP_H_ */
