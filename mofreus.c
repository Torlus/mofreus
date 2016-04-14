#include <stdlib.h>
#include <stdio.h>

#define MAX_SIZE (4L * 1024L * 1024L)
#define MRU_COUNT 2

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
  char span_char, c;

  if (size <= 2 + MRU_COUNT)
    return 0;

  // if (src[0] != 'B' || src[1] != 'M')
  //  return 0;

  // Initialize Usage Count Per Character (UCPC)
  for(n = 0; n < 256; n++) {
    uses[n] = 0;
  }
  for(n = 0; n < MRU_COUNT; n++) {
    top_k[n] = 0;
    top_v[n] = 0L;
  }
  // Collect UCPC on source file
  for(sp = 2 + MRU_COUNT; sp < size; sp++) {
    uses[0xff & src[sp]]++;
  }
  // Bubble-sort UCPC
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
  // Generate a LUT for RLE-enabled characters
  for(n = 0; n < 256; n++) {
    uses[n] = 0;
  }
  for(n = 0; n < MRU_COUNT; n++) {
    dst[2 + n] = top_k[n];
    uses[0xff & top_k[n]] = 1;
  }
  dp = 2 + MRU_COUNT;
  c = src[0];
  span_char = c;
  span_size = uses[0xff & c];
  for(sp = 2; sp < size; sp++) {
    c = src[sp];
    if ((span_char == c) && (span_size > 0) && (sp != size - 1)) {
      span_size++;
    } else {
      dst[dp++] = span_char;
      if (dp >= size)
        return 0;
      if (span_size > 0) {
        // Variable length encoding, LE, 7 bits & MSB as as "continue" marker
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
      }
      span_char = c;
      span_size = uses[0xff & c];
    }
  }
  // src[0] = 'C';
  return dp;
}

size_t mofreus_uncompress(size_t size, char *src, char *dst) {
  size_t sp, dp;
  int n, i, k;
  long span_size;
  char span_char, c;

  if (size <= 2 + MRU_COUNT)
    return 0;

  // if (src[0] != 'C' || src[1] != 'M')
  //  return 0;

  // Generate a LUT for RLE-enabled characters
  for(n = 0; n < 256; n++) {
    uses[n] = 0;
  }
  for(n = 0; n < MRU_COUNT; n++) {
    uses[0xff & src[n + 2]] = 1;
  }
  dp = 2;
  for(sp = 2 + MRU_COUNT; sp < size; sp++) {
    c = dst[dp] = src[sp];
    dp++;
    if (uses[0xff & c] && (sp != size - 1)) {
      span_char = c;
      c = src[++sp];
      span_size = 0x7f & c;
      k = 0;
      while((0x80 & c) != 0) {
        c = src[++sp];
        k += 7;
        span_size |= (0x7f & c) << k;
      }
      for(n = 1; n < span_size; n++)
        dst[dp++] = span_char;
    }
  }
  return dp;
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
  if (comp_size > 0) {
    uncp_size = mofreus_uncompress(comp_size, target, verify);
    printf("Uncompressed size: %ld\n", uncp_size);
    if (size == uncp_size) {
      printf("Ratio: %02ld%%\n", comp_size * 100L / size);
    }
  }

  return 0;
}
