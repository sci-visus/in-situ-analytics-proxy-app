/*
 * Patch.h
 *
 *  Created on: Feb 5, 2012
 *      Author: bremer5
 */

#ifndef PATCH_H_
#define PATCH_H_

#include <vector>
#include "PointIndex.h"
#include "Box.h"


//! A patch represents k fields of data covering the given box
class Patch
{
public:

  //! Default constructor creating an invalid and empty patch
  Patch();

  //! Construct a patch from given arrays covering the given box
  Patch(const std::vector<FunctionType*>& fields, const SubBox& box) : mFields(fields), mDomain(box) {};

  //! Destructor
  ~Patch() {}

  //! Return a pointer to the k-th field
  const FunctionType* field(int k) const {return mFields[k];}

  //! Return the value of the k-th field at position p
  FunctionType operator()(const PointIndex& p, int k) const;

  //! Return the value of the first field at position p
  FunctionType operator()(const PointIndex& p) const;

  //! operator=
  Patch operator=(const Patch & p) {
    if(&p != this) {
      mFields = p.mFields;
    }
    return *this;
  }


  SubBox& domain()  { return mDomain; }
private:

  //! The pointers to k fields of values
  std::vector<FunctionType*> mFields;

  //! The box describing the domain covered by the fields
   SubBox mDomain;
};



#endif /* PATCH_H_ */
