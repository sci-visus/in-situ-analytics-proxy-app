/*
 * TopoOutputStream.h
 *
 *  Created on: Jan 30, 2012
 *      Author: bremer5
 */

#ifndef TOPOOUTPUTSTREAM_H_
#define TOPOOUTPUTSTREAM_H_

#include <vector>
#include "Token.h"
#include "TokenOutputBuffer.h"

//! A TopoOutputStream defines the streaming output API
class TopoOutputStream {

public:

  //! The default message size
  static const uint32_t sDefaultMessageSize;

  //! Default constructor
  TopoOutputStream(const std::vector<GraphID>& destinations,uint32_t message_size = sDefaultMessageSize);

  //! Destructor
  virtual ~TopoOutputStream() {}

  //! Write a token to the stream
  TopoOutputStream& operator<<(const Token::VertexToken& t) {return write<Token::VertexToken>(t);}
  TopoOutputStream& operator<<(const Token::EdgeToken& t) {return write<Token::EdgeToken>(t);}
  TopoOutputStream& operator<<(const Token::FinalToken& t) {return write<Token::FinalToken>(t);}
  TopoOutputStream& operator<<(const Token::SegToken& t) {return write<Token::SegToken>(t);}
  TopoOutputStream& operator<<(const Token::EmptyToken& t) {return write<Token::EmptyToken>(t);}

  //! Flush the remaining buffer by calling a write
  int flush();

protected:

  //! The list of graphs which are the destinations of our messages
  const std::vector<GraphID> mDestinations;

  //! The token buffer
  TokenOutputBuffer mBuffer;


private:

  //! The internal write function to be re-implemented
  /*! The write function used by the operator to actually pass on
   *  the latest buffer. The downstream function is responsible for
   *  copying the buffer if necessary since once this function
   *  returns the stream will reclaim the buffer.
   * @param buffer: Pointer to the start of the buffer
   * @param size: Number of bytes to write
   * @return 1 if successful; 0 otherwise.
   */
  virtual int write(const char* buffer, uint32_t size) = 0;

  template <class TokenType>
  TopoOutputStream& write(const TokenType& token);
};

template <class TokenType>
TopoOutputStream& TopoOutputStream::write(const TokenType& token)
{
  // If we have enough space in our buffer store the token and return
  if (mBuffer.push<TokenType>(token))
    return *this;

  // Otherwise, prepare the buffer for writing
  mBuffer.compactify();

  // Pass it on
  write(mBuffer.buffer(),mBuffer.size());

  // and clear it
  mBuffer.reset();

  // Try storing the token again
  bool success = mBuffer.push<TokenType>(token);

  sterror(!success,"Could not write token even after clearing the buffer");

  return *this;
}



#endif /* TOPOOUTPUTSTREAM_H_ */
