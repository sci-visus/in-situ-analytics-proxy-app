#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

using namespace std;

int main(int argc, char** argv) {

  if (argc < 11) {
    printf( "Usage: %s inputfile gXDim gYDim gZDim blockDimX blockDimY \
    bloxkDimZ offsetX offsetY offsetZ  [output.raw]\n", argv[0]);
    return 0;
  }

  int dim = 3;
  int tot_blocks;
  int data_size_x = atoi(argv[2]);
  int data_size_y = atoi(argv[3]);
  int data_size_z = atoi(argv[4]);
  int block_dim_x = atoi(argv[5]);
  int block_dim_y = atoi(argv[6]);
  int block_dim_z = atoi(argv[7]);
  int offset_x = atoi(argv[8]);
  int offset_y = atoi(argv[9]);
  int offset_z = atoi(argv[10]);

  const char* outputfile;
  if(argv[11])
    outputfile = argv[11];
  else
    outputfile = "output_block.raw";
  
  int block_offset_x = (offset_x / block_dim_x) * data_size_x;
  int block_offset_y = (offset_y / block_dim_y) * data_size_y;
  int block_offset_z = (offset_z / block_dim_z) * data_size_z;

  char* input_file = argv[1];
  FILE *fp = fopen(input_file, "rb");

  float *buffer = (float*)malloc(sizeof(float)*block_dim_x*block_dim_y*
                                               block_dim_x);
  float *ptr = buffer;
  for (int k=0; k<block_dim_z; k++) {
    for (int j=0; j<block_dim_y; j++) {
      fseek(fp, data_size_y*data_size_x*(block_offset_z+k) +
                data_size_x*(block_offset_y+j) + block_offset_x, SEEK_SET);
      fread((void*)ptr, sizeof(float), block_dim_x, fp);
      ptr = ptr+block_dim_x;
    }
  }
  fclose(fp);

  fp = fopen(outputfile, "wb");
  fwrite((const void*)buffer, sizeof(float), 
          block_dim_x*block_dim_y*block_dim_z, fp);
  fclose(fp);
  free(buffer);
  buffer = NULL;
  ptr = NULL;

  return 0;
}
;

