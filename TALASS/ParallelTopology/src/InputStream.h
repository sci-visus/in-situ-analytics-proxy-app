/*
 * InputStream.h
 *
 *  Created on: Mar 13, 2014
 *    Author  : landge1
 *
 */
 
 #ifndef INPUTSTREAM_H
 #define INPUTSTREAM_H

 #include <vector>

 //! Base class for all input streams
 
 class InputStream {
 
  public:

    InputStream() {}

    virtual ~InputStream() {}

    virtual int push(char* buff) = 0;
 };
 
 #endif /*INPUTSTREAM_H*/
