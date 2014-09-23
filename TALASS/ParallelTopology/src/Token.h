/*
 * Token.h
 *
 *  Created on: Jan 31, 2012
 *      Author: bremer5
 */

#ifndef TOKEN_H_
#define TOKEN_H_

#include <vector>
#include <string.h>
#include "DistributedDefinitions.h"

typedef int8_t MultiplicityType;

namespace Token {

//! The number of recognized tokens
#define NUM_TOKENS 6

//! All recognized token types
enum TokenType {
  VERTEX    = 0,
  EDGE      = 1,
  FINAL     = 2,
  SEG       = 3,
  EMPTY     = 4,
};



class FinalToken
{
public:

  //! Default constructor
  FinalToken(GlobalIndexType i=GNULL) : mIndex(i) {}

  //! Destructor
  ~FinalToken() {}

  //! Return the raw index without the type bits
  GlobalIndexType index() const {return mIndex;}

  //! Set the raw index which sets the type bits
  void index(GlobalIndexType i) {mIndex = i;}

  //! Return the size in bytes of this token
  uint32_t size() const {return sizeof(FinalToken);}

  //! The type of this token
  TokenType type() const {return FINAL;}

private:

  //! The id of the vertex
  GlobalIndexType mIndex;
};

class EmptyToken
{
public:

  //! Return the size in bytes of this token
  uint32_t size() const {return 0;}

  //! The type of this token
  TokenType type() const {return EMPTY;}
};


class VertexToken
{
public:

  //! Default constructor
  VertexToken(GlobalIndexType i=GNULL, FunctionType f=0, MultiplicityType mult=1, MultiplicityType out=1);

  //! Destructor
  ~VertexToken() {}

  //! Return the raw index without the type bits
  GlobalIndexType index() const {return mIndex;}

  //! Set the raw index which sets the type bits
  void index(GlobalIndexType i) {mIndex = i;}

  //! Return the multiplicity
  MultiplicityType multiplicity() const {return mMultiplicity;}

  //! Return the number of copies this token represents
  MultiplicityType outstanding() const {return mOutstanding;}

  //! Set the multiplicity
  void multiplicity(MultiplicityType mult) {mMultiplicity = mult;}

  //! Set the number of copies
  void outstanding(MultiplicityType out) {mOutstanding = out;}

  //! value of the data object
  FunctionType value() const {return mValue;}

 //! value of the data object
  void value(FunctionType value) {mValue = value;}

  //! Return the size in bytes of this token
  uint32_t size() const {return sizeof(VertexToken);}

  //! The type of this token
  TokenType type() const {return VERTEX;}

private:

  //! The id of the vertex
  GlobalIndexType mIndex;

  //! The data
  FunctionType mValue;

  //! The multiplicity
  MultiplicityType mMultiplicity;

  //! The number of outstanding tokens
  MultiplicityType mOutstanding;
};

//! Token to encode an edge between to vertices
class EdgeToken
{
public:

  //! Default constructor
  EdgeToken(GlobalIndexType v0=GNULL, GlobalIndexType v1=GNULL);

  //! Destructor
  ~EdgeToken() {}

  //! Access operator (note that it does not return a reference)
  GlobalIndexType operator[](int i) const {return mIndex[i];}

  //! Set the source
  void source(GlobalIndexType index) {mIndex[0] = index;}

  //! Set the destination
  void destination(GlobalIndexType index) {mIndex[1] = index;}

  //! Determine the size
  uint32_t size() const {return sizeof(EdgeToken);}

  //! The type of this token
  TokenType type() const {return EDGE;}

public:

  //! The two edge indices
  GlobalIndexType mIndex[2];
};


//! Token to encode the segmentation information of a vertices
class SegToken
{
public:

  //! Default constructor
  SegToken(GlobalIndexType id, GlobalIndexType seg, FunctionType f);

  //! Destructor
  ~SegToken() {}

  //! Return the vertex id
  GlobalIndexType id() const {return mId;}

  //! Set the vertex id
  void id(GlobalIndexType i) {mId = i;}

  //! Return the segmentation id
  GlobalIndexType seg() const {return mSeg;}

  //! Set the segmentation id
  void seg(GlobalIndexType s) {mSeg = s;}

  //! Return the function value
  FunctionType f() const {return mFunction;}

  //! Set the function value
  void f(FunctionType func) {mFunction = func;}

  //! Determine the size
  uint32_t size() const {return sizeof(SegToken);}

  //! The type of this token
  TokenType type() const {return SEG;}

public:

  //! The vertex id
  GlobalIndexType mId;

  //! The segmentation id
  GlobalIndexType mSeg;

  //! The function value
  FunctionType mFunction;
};





}

#endif /* TOKEN_H_ */
