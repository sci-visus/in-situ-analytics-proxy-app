#include<cstdio>
#include<cstdlib>


int main(int argc, char **argv) {
  
  if(argc != 4) {
    printf("Usage: %s x y z\n", argv[0]);
    return 0;
  }
  int x = atoi(argv[1]);
  int y = atoi(argv[2]);
  int z = atoi(argv[3]);

  float* data = new float[x*y*z];
  int count=0;
  for(int k = 0; k < z; k++) {
    for(int j = 0; j < y; j++) {
      for(int i = 0; i < x; i++) {
        data[count] = count++;
      }
    }
  }
  char fname[256];
  sprintf(fname, "data_%d_%d_%d.raw", x, y, z);
  FILE* datafile = fopen(fname, "wb");
  fwrite(data, sizeof(float), x*y*z, datafile);
  fclose(datafile);
  return 1;
}

