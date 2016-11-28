/*
 * cpu & mem eater app.
 * To test docker and kubernetes limits.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
//  char *ap[1024];
  char *ap[3];
  unsigned long long chunk_size = 512*1024*1024;

  setbuf(stdout, NULL);

  printf("start\n");

  unsigned i=0;
  while (i < sizeof(ap)/sizeof(char*)) {
    ap[i] = malloc(chunk_size);
    if (ap[i]) {
      printf("allocated %d bytes, total %.1f GB\n",
        chunk_size, (float)(chunk_size+i*chunk_size)/(1024*1024*1024));
      memset(ap[i], i, chunk_size);
    } else {
      printf("can't allocate %ul bytes\n", chunk_size);
      break;
    }
    i++;
  }
  printf("sleeping\n");
  sleep(600);

  printf("end\n");
  return 0;
}
