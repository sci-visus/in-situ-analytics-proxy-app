/*
 * SortedInputStream.h
 *
 *  Created on: Mar 13, 2014
 *      Author: landge1
 */

#ifndef SORTEDINPUTSTREAM_H
#define SORTEDINPUTSTREAM_H

#include <vector>
#include <queue>
#include <deque>
#include "Token.h"
#include "TokenInputBuffer.h"
#include "InputStream.h"

 enum TokenType {
   VERTEX    = 0,
   EDGE      = 1,
   FINAL     = 2,
   SEG       = 3,
   EMPTY     = 4,
 };

//! A SortedInputStream defines the streaming input API
class SortedInputStream : public InputStream
{

public:

  //! A const iterator to read tokens
  class iterator {
  public:

    //! Friend declaration to allow access to private members
    friend class SortedInputStream;

    //! Default iterator
    iterator();

    //! Advance the iterator
    iterator& operator++(int i);

    //! Compare iterators
    bool operator==(const iterator& it) const;

    //! Compare iterators
    bool operator!=(const iterator& it) const;

    //! Return the type of the current token
    Token::TokenType type() const {return mIt.type();}

    //! Return a reference to the current token
    operator const Token::VertexToken&() const {return mIt.reference<Token::VertexToken>();}

    //! Return a reference to the current token
    operator const Token::EdgeToken&() const {return mIt.reference<Token::EdgeToken>();}

    //! Return a reference to the current token
    operator const Token::EmptyToken&() const {return mIt.reference<Token::EmptyToken>();}

    //! Return a reference to the current token
    operator const Token::FinalToken&() const {return mIt.reference<Token::FinalToken>();}

    //! Return a reference to the current token
    operator const Token::SegToken&() const {return mIt.reference<Token::SegToken>();}

  private:

    //! Private constructor to prevent users from accessing it
    iterator(SortedInputStream* stream);

    //! Return a refernce to the current token
    template <class TokenClass>
    const TokenClass& reference() const {return mIt.reference<TokenClass>();}

    //! The pointer to the corresponding input stream
    SortedInputStream* mStream;

    //! The iterator into the current buffer
    TokenInputBuffer::iterator mIt;

    //! The end of the current buffer
    TokenInputBuffer::iterator mEnd;
  };


  //! Default constructor
  SortedInputStream(uint32_t outstanding);

  //! Destructor
  virtual ~SortedInputStream();

  //! Return an iterator pointing to the first token
  iterator begin() {return iterator(this);}

  //! Return an iterator pointing beyond the last token
  iterator end() {return iterator();}

  //! Return the number of outstanding EMPTY tokens
  uint32_t outstanding() const {return mOutstanding;}

  //! Report whether you have all the messages you are expecting
  bool complete() const {return (mOutstanding == 0);}

  //! Push the given buffer onto the stream
  /*! Push the message represented by the buffer onto the stream.
   *  Note that the stream will take ownership of the buffer and
   *  delete it using "delete[]" once it has been parsed.
   * @param buffer
   * @return 1 if successful; 0 otherwise
   */
  virtual int push(char* buffer);

  //! Return the type of the last token in the last buffer pushed
  Token::TokenType lastType() const;

protected:

  //! Queue to hold iterators of edge tokens
  std::queue<TokenInputBuffer::iterator> mEdgeQ;

  TokenInputBuffer::iterator *mEdgeIter;

  TokenInputBuffer::iterator *mHighVertex;

  void initializePriorityQ();

  struct compareVertexTokens {
    bool operator() (TokenInputBuffer::iterator it_A,
        TokenInputBuffer::iterator it_B) {
      const Token::VertexToken& v_A = it_A.reference<Token::VertexToken>();
      const Token::VertexToken& v_B = it_B.reference<Token::VertexToken>();

      return (v_A.value() == v_B.value()) ? v_A.index() < v_B.index() :
        v_A.value() < v_B.value();
    }
  };

  //! Priority queue of all highest vertices from each message
  std::priority_queue<TokenInputBuffer::iterator,
                      std::vector<TokenInputBuffer::iterator>,
                      compareVertexTokens> mVertexPriorityQ;

  //! Return a pointer to the top message
  TokenInputBuffer* front();

  //! Return the highest vertex from the vertex priority queue
  TokenInputBuffer::iterator highVertex();

  //! Pull more data
  virtual int pull() {sterror(true,"This input stream cannot pull data."); return 0;}

  //! The fifo buffer of TokenInputBuffers
  std::deque<TokenInputBuffer*> mMessages;

  //! The number of EMPTY tokens still to come
  uint32_t mOutstanding;
};

#endif /* SORTEDINPUTSTREAM_H_ */
