// Copyright (c) C 2015, copied from https://github.com/porglezomp/wav-synth

#include "wavwriter.hpp"

#include <stdio.h>

#define SIZE_OF_HEADER 36

void write(FILE* f, int size, int32_t arg)
{
  uint8_t x;
  int16_t y;
  int32_t z;

  switch (size) {
  case 1:
    x = (uint8_t) arg;
    fwrite(&x, size, 1, f);
    break;
  case 2:
    y = (int16_t) arg;
    fwrite(&y, size, 1, f);
    break;
  case 4:
    z = (int32_t) arg;
    fwrite(&z, size, 1, f);
    break;
  }
}

void writewav(float *data, int num_channels, int samples, int sample_rate, const char* filename) {
  FILE* f = fopen(filename, "w");
  // We multiply the number of samples by 4 each float takes up 4 bytes, so 
  // we will need to write out 4 * number of samples.
  /* header*/
  fputs("RIFF", f);                              /* main chunk       */
  write(f, 4, SIZE_OF_HEADER + samples * 4);     /* chunk size       */
  fputs("WAVE", f);                              /* file format      */
  fputs("fmt ", f);                              /* format chunk     */
  write(f, 4, 16);                               /* size of subchunk */
  write(f, 2, 3);                                /* format (1 = PCM, 3 = IEEE float) */
  write(f, 2, num_channels);                     /* # of channels    */
  write(f, 4, sample_rate);                      /* sample rate      */
  write(f, 4, sample_rate);                      /* byte rate        */
  write(f, 2, 1);                                /* block align      */
  write(f, 2, 32);                               /* bits per sample (8 for 8bit PCM, 32 for floats) */
  fputs("data", f);                              /* data chunk       */
  /* body */
  fwrite(data, 1, samples * 4, f);               /* actual audio     */

  fclose(f);
}
