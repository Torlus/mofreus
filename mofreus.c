#include <stdlib.h>
#include <stdio.h>

#define MAX_SIZE (64L * 1024L)
#define MRU_COUNT 4

char source[MAX_SIZE];
char target[MAX_SIZE];
char verify[MAX_SIZE];

static long uses[256];
static long top_v[MRU_COUNT];
static char top_k[MRU_COUNT];

size_t mofreus_compress(size_t size, char *src, char *dst) {
  size_t sp, dp;
  int n, i, k;
  long span_size;
  char span_char;

  if (size <= 2 + MRU_COUNT)
    return 0;

  for(n = 0; n < 256; n++) {
    uses[n] = 0;
  }
  for(sp = 0; sp < size; sp++) {
    uses[(unsigned char)src[sp]]++;
  }
  for(n = 0; n < MRU_COUNT; n++) {
    top_k[n] = 0;
    top_v[n] = 0L;
  }
  // Bubble sort
  for(n = 0; n < 256; n++) {
    k = -1;
    for(i = 0; i < MRU_COUNT; i++) {
      if (top_v[i] < uses[n])
        k = i;
    }
    if (k >= 0) {
      for(i = 0; i <= k - 1; i++) {
        top_k[i] = top_k[i + 1];
        top_v[i] = top_v[i + 1];
      }
      top_k[k] = n;
      top_v[k] = uses[n];
    }
  }
  for(n = 0; n < 256; n++) {
    uses[n] = 0;
  }
  for(n = 0; n < MRU_COUNT; n++) {
    uses[(unsigned char)top_k[n]] = 1;
  }
  dp = 2 + MRU_COUNT;
  span_char = src[0];
  span_size = uses[(unsigned char)src[0]];
  for(sp = 0; sp < size; sp++) {
    if ((span_char == src[sp]) && (span_size > 0)) {
      span_size++;
    } else {
      dst[dp++] = span_char;
      if (dp >= size)
        return 0;
      while (span_size > 0) {
        if (span_size > 0x7f) {
          dst[dp++] = 0x80 | (span_size & 0x7f);
        } else {
          dst[dp++] = span_size;
        }
        span_size >>= 7;
        if (dp >= size)
          return 0;
      }
      span_char = src[sp];
      span_size = uses[(unsigned char)src[sp]];
    }
  }
  return dp;
}

size_t mofreus_uncompress(size_t size, char *src, char *dst) {
  return 0;
}


int main(int argc, char *argv[]) {
  FILE *f;
  size_t size, comp_size, uncp_size;

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
    fprintf(stderr, "File %s is too large (max: %ld).\n", argv[1], MAX_SIZE);
    fclose(f);
    return -2;
  }
  fseek(f, 0L, SEEK_SET);
  fread(source, size, 1, f);
  fclose(f);

  printf("Source size: %ld\n", size);
  comp_size = mofreus_compress(size, source, target);
  printf("Compressed size: %ld\n", comp_size);
  if (comp_size < size) {
    uncp_size = mofreus_uncompress(comp_size, target, verify);
    printf("Uncompressed size: %ld\n", uncp_size);
  }

  return 0;
}
