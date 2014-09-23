/*
 * LinearScatter.cpp
 *
 *  Created on: Feb 14, 2012
 *      Author: bremer5
 */

#include "ModuloScatter.h"

ModuloScatter::ModuloScatter(uint32_t sink_count, uint32_t factor) : ModuloFlow(sink_count,factor)
{
}

void ModuloScatter::scatterSinks(uint32_t source) {

  std::vector<uint32_t> tsinks = sinks(source);
  std::vector<uint32_t>::iterator it;
  if (!tsinks.empty()) {
    for (it = tsinks.begin(); it < tsinks.end(); it++) {
      scatterSinks(*it);
    }
  }
  else {
    mScatterSink.push_back(source);
  }

}

