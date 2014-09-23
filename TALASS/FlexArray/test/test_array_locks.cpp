#include "ArrayLocks.h"

using namespace FlexArray;

int main(void)
{
  ArrayLocks array;

  array.resize(10);

  fprintf(stderr,"Lock 0:  %d\n",(int)array[0]);

  array.lock(0);

  fprintf(stderr,"Lock 0:  %d\n",(int)array[0]);

  //array.lock(0);

  fprintf(stderr,"Lock 0:  %d\n",(int)array[0]);

  array.unlock(0);

  fprintf(stderr,"Lock 0:  %d\n",(int)array[0]);


          

  return 1;
}

