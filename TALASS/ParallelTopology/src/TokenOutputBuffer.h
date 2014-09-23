/*
 * TokenBuffer.h
 *
 *  Created on: Jun 16, 2012
 *      Author: bremer5
 */

#ifndef TOKENBUFFER_H_
#define TOKENBUFFER_H_

#include <vector>
#include "Token.h"

/*! A TokenOutputBuffer collects tokens in a local buffer to be send
 *  in a single void* type message. For efficiency reasons, the data
 *  is initially collected in two buffers, one for the tokens and one
 *  for the types. Once a messages is compactified, i.e. made ready to
 *  ship the types are copied to the end of the token buffer and the
 *  complete buffer can be send. In addition to this payload we also
 *  store two additional 32-bit integer and a GraphID. The first integer at the very
 *  beginning indicating the size of the token buffer; Then we add the GraphID 
 *  which is the GraphID of the destination and; The second integer right
 *  after the token buffer indicating the number of tokens present
 *  which is equivalent to the size of the type portion of the buffer.
 *
 *  <uint32_t> <GraphID> <upstream/downstream> <token0> .... <tokenN> <uint32_t> <type0> .... <typeN>
 */
class TokenOutputBuffer
{
public:

  //! Default constructor restricting the sizes
  TokenOutputBuffer(uint32_t max_size);

  //! Destructor
  ~TokenOutputBuffer();

  //! Reset the buffer
  void reset();

  //! The current message size
  uint32_t size() const;

  //! The destination of the current message
  GraphID destination() const;

  //! The direction of the message : upstream/downstream
  uint8_t direction() const;

  //! The current number of tokens
  uint32_t count() const {return mTypeBuffer.size();}

  //! Return the internal buffer will return NULL if not compact
  const char* buffer() const;

  //! Compactify the buffer. Note that this will prevent
  //! any further writes.
  void compactify();

  //! Push a token onto the buffer
  /*! Push a token onto the buffer if there is enough space.
   *  If there is not enough space or there are too many
   *  tokens on the stream simply return
   *  @param token: The token to be stored
   *  @return 1 if successful; 0 otherwise
   */
  template <class TokenClass>
  int push(const TokenClass& t);

private:

  //! The raw buffer
  char *mBuffer;

  //! The position in the buffer to write the next token
  char* mToken;

  //! The buffer size of the entire buffer
  uint32_t mCapacity;

  //! A flag indicating whether the buffer is compactifed
  bool mCompact;

  //! The temporary buffer storing the types
  std::vector<char> mTypeBuffer;

  //! The head of the next token type
  char *mType;
};


template <class TokenClass>
int TokenOutputBuffer::push(const TokenClass& t)
{
  if (size()+t.size() > mCapacity)
    return 0;

  //fprintf(stderr,"Pushing token %d\n",t.type());
  mTypeBuffer.push_back(t.type());

  memcpy(mToken,&t,t.size());
  mToken += t.size();

  mCompact = false;

  return 1;
}


#endif /* TOKENBUFFER_H_ */
