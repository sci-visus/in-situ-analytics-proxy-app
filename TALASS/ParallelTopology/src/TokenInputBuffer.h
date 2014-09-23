/*
 * TokenInputBuffer.h
 *
 *  Created on: Jun 17, 2012
 *      Author: bremer5
 */

#ifndef TOKENINPUTBUFFER_H_
#define TOKENINPUTBUFFER_H_

#include "Token.h"

class TokenInputBuffer
{
public:

  //! Const iterator to process tokens
  class iterator {
  public:

    //! Friend declaration to allow access to private comnstructor
    friend class TokenInputBuffer;

    //! Empty constructor
    iterator() : mSize(0),mToken(NULL),mCount(0),mTypes(NULL),mIndex(0) {}

    //! Copy constructor
    iterator(const iterator& it);

    //! Advance the iterator
    iterator& operator++(int i);

    //! Comparison operator
    bool operator==(const iterator it) const;

    //! Comparison operator
    bool operator!=(const iterator it) const;

    //! Return a reference to the current token
    template <class TokenClass>
    const TokenClass& reference() const {return reinterpret_cast<const TokenClass&>(*mToken);}

    //! Return the type of the current token
    Token::TokenType type() const {return (Token::TokenType)(mTypes[mIndex]);}

//  private:

    //! Private constructor to prevent the user from calling it
    iterator(const char* buffer, bool fast_forward);


    template <class TokenClass>
    const TokenClass* pointer() const {return reinterpret_cast<const TokenClass*>(mToken);}

    //! The size of the token buffer
    uint32_t mSize;

    //! The destination GraphID of the buffer
    GraphID mDest;

    //! The direction of the message : downstream/upstream
    uint8_t mDirection;

    //! The head of the next token
    const char* mToken;

    //! The number of tokens
    uint32_t mCount;

    //! The head of the types buffer
    const uint8_t* mTypes;

    //! The current count
    uint32_t mIndex;

  };


  //! Construct an input buffer from a piece of memory
  /*! "Construct" a token buffer from an existing piece of memory.
   *  The TokenInputBuffer will assume the pointer points to a valid
   *  compactified TokeOutputBuffer object, assume ownership of the
   *  buffer and delete it at exit using delete[].
   */
  TokenInputBuffer(const char* buffer) : mBuffer(buffer) {}

  //! Destructor
  ~TokenInputBuffer() {delete[] mBuffer;}

  //! Return an iterator pointing to the first token
  iterator begin() const {return iterator(mBuffer,false);}

  //! Return an iterator pointing beyond the last token
  iterator end() const {return iterator(mBuffer,true);}

  //! Return the type of the last token of the buffer
  Token::TokenType lastType() const;

private:

  //! Pointer to the head of the buffer
  const char* mBuffer;
};


#endif /* TOKENINPUTBUFFER_H_ */
