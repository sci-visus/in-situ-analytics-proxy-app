#include "IDXArray.h"

using namespace FlexArray;

int main(int argc, const char* argv[])
{
  if (argc < 2) {
    fprintf(stderr,"Usage: %s <filename.idx>\n",argv[0]);
    return 0;
  }

  IDXArray<float> data(argv[1]);




  return 1;
}



