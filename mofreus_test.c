#include <stdlib.h>
#include <stdio.h>

extern int mofreus_compress(int size, char src[], char dst[]);
extern int mofreus_uncompress(int size, char src[], char dst[]);

#define MAX_SIZE (4 * 1024 * 1024)

char source[MAX_SIZE];
char target[MAX_SIZE];
char verify[MAX_SIZE];

int main(int argc, char *argv[]) {
  FILE *f;
  int size, comp_size, uncp_size;
  int n;

  if (argc <= 1) {
    fprintf(stderr, "Usage: %s <file>", argv[0]);
    return 1;
  }

  f = fopen(argv[1], "r");
  if (f == NULL) {
    fprintf(stderr, "Could not open %s for reading.\n", argv[1]);
    return -1;
  }
  fseek(f, 0L, SEEK_END);
  size = ftell(f);
  if (size > MAX_SIZE) {
    fprintf(stderr, "File %s is too large (max: %d).\n", argv[1], MAX_SIZE);
    fclose(f);
    return -2;
  }
  fseek(f, 0L, SEEK_SET);
  fread(source, size, 1, f);
  fclose(f);

  printf("Source size: %d\n", size);
  comp_size = mofreus_compress(size, source, target);
  printf("Compressed size: %d\n", comp_size);

  if (comp_size > 0) {
    uncp_size = mofreus_uncompress(comp_size, target, verify);
    printf("Uncompressed size: %d\n", uncp_size);
    if (size != uncp_size) {
      printf("File size mismatch!\n");
      return -3;
    }
    printf("Ratio: %02d%%\n", comp_size * 100 / size);
    for(n = 0; n < size; n++) {
      if (source[n] != verify[n]) {
        printf("Diff at position #%d!\n", n);
        return -4;
      }
    }
  }
  printf("Done!\n");
  return 0;
}
