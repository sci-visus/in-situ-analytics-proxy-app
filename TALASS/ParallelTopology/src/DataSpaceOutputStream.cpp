/*
 * DataSpaceOutputStream.cpp
 *
 *  Created on: Apr 4, 2012
 *      Author: bremer5
 */

#include <cstdlib>
#include "DataSpaceOutputStream.h"
#include "DataSpaceInputStream.h"

DataList DataSpaceOutputStream::sDataList;

DataSpaceOutputStream::DataSpaceOutputStream(uint32_t rank)
: TopoOutputStream(std::vector<GraphID>(), (uint32_t)-1), mRank(rank)
{
}

//! Function to write .dot file to view Output Stream
int DataSpaceOutputStream::write_to_dot(FILE* output)
{

  fprintf(output,"digraph G {\n");



  //DataList::iterator it;
  //for (it=sDataList.begin();it!=sDataList.end();it++) {

    DataSpaceInputStream input(sDataList.front());
    DataSpaceInputStream::iterator it;

    for (it=input.begin();it!=input.end();it++) {

      switch (it.type()) {
        case Token::EDGE: {
          const Token::EdgeToken& e = it;
          fprintf(output,"%d -> %d\n",e[0],e[1]);
          break;
        }
        default:
          break;
      }
    }

  fprintf(output,"\n}\n");

  return 1;
}

int DataSpaceOutputStream::write(const char* buffer, uint32_t size)
{
  DataElement data;

  // First we fill in the information for the descriptor
  data.descriptor.size = size;
  data.descriptor.rank = mRank;
  data.descriptor.timeStep = 0;
  data.descriptor.type = 0;

  // The we copy the data (this is only a test after all)
  data.data = new char[size];
  memcpy(data.data,buffer,size);

  // And push the element onto the global list
  sDataList.push_front(data);

  return 1;
}

