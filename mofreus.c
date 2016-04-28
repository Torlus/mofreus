#ifdef JAVA
package com.torlus.util;

public class Mofreus {
#endif

#ifdef JAVA
#define PUBLIC public static
#define PRIVATE private static
#define ARRAY_INST(type, name, length) type name [] = new type [ length ]
#define CONST(type, name, value) final type name = value
#else
#define byte char
#define PUBLIC
#define PRIVATE static
#define ARRAY_INST(type, name, length) type name [ length ]
#define CONST(type, name, value) const type name = value
#endif

PRIVATE CONST(int, MRU_COUNT, 4);

PRIVATE ARRAY_INST(int, uses, 256);
PRIVATE ARRAY_INST(int, top_v, MRU_COUNT);
PRIVATE ARRAY_INST(byte, top_k, MRU_COUNT);

PUBLIC int mofreus_compress(int size, byte src[], byte dst[]) {
  int sp, dp;
  int n, i, k;
  int span_size;
  byte span_char, c;

  if (size <= 2 + MRU_COUNT)
    return 0;

  // if (src[0] != 'B' || src[1] != 'M')
  //  return 0;

  // Initialize Span Use Size Improvement
  for(n = 0; n < 256; n++) {
    uses[n] = 0;
  }
  for(n = 0; n < MRU_COUNT; n++) {
    top_k[n] = 0;
    top_v[n] = -(size + 2 + MRU_COUNT);
  }
  span_char = 0;
  uses[span_char] = (span_char == src[2 + MRU_COUNT] ? 0 : 1);
  // Collect SUSI on source file
  for(sp = 2 + MRU_COUNT; sp < size; sp++) {
    c = src[sp];
    if (c == span_char) {
      uses[0xff & span_char]++;
    } else {
      uses[0xff & span_char]--;
      span_char = c;
    }
  }
  // Bubble-sort SUSI
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
      top_k[k] = (byte)n;
      top_v[k] = uses[n];
    }
  }
  // Keep only entries where SUSI > 0
  if (top_v[MRU_COUNT - 1] < 2 + MRU_COUNT)
    return 0;
  for(i = 0; i < MRU_COUNT - 1; i++) {
    if (top_v[i] <= 0) {
      top_k[i] = top_k[MRU_COUNT - 1];
      top_v[i] = top_v[MRU_COUNT - 1];
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
  c = src[2];
  span_char = c;
  span_size = uses[0xff & c];
  for(sp = 3; sp <= size; sp++) {
    c = src[sp];
    if ((span_char == c) && (span_size > 0) && (sp != size)) {
      span_size++;
    } else {
      dst[dp++] = span_char;
      if (dp >= size)
        return 0;
      // Variable length encoding, LE, 7 bits & MSB as as "continue" marker
      while (span_size > 0) {
        if (span_size > 0x7f) {
          dst[dp++] = (byte)(0x80 | (span_size & 0x7f));
        } else {
          dst[dp++] = (byte)span_size;
        }
        span_size >>= 7;
        if (dp >= size)
          return 0;
      }
      span_char = c;
      span_size = uses[0xff & c];
    }
  }
  // dst[0] = 'C'; dst[1] = 'M';
  dst[0] = src[0]; dst[1] = src[1];
  return dp;
}

PUBLIC int mofreus_uncompress(int size, byte src[], byte dst[]) {
  int sp, dp;
  int n, i, k;
  int span_size;
  byte span_char, c;

  if (size <= 2 + MRU_COUNT)
    return 0;

  // if (src[0] != 'C' || src[1] != 'M')
  //  return 0;

  // Generate a LUT for RLE-enabled characters
  for(n = 0; n < 256; n++) {
    uses[n] = 0;
  }
  for(n = 0; n < MRU_COUNT; n++) {
    uses[0xff & src[2 + n]] = 1;
  }
  dp = 2;
  for(sp = 2 + MRU_COUNT; sp < size; sp++) {
    c = src[sp];
    dst[dp++] = c;
    if ((uses[0xff & c] != 0) && (sp != size - 1)) {
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
  // dst[0] = 'B'; dst[1] = 'M';
  dst[0] = src[0]; dst[1] = src[1];
  return dp;
}

#ifdef JAVA
}
#endif
