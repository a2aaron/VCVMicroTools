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

void writewav(uint8_t *data, int samples, int sample_rate, const char* filename)
{
  FILE* f = fopen(filename, "w");

  /* header*/
  fputs("RIFF", f);                              /* main chunk       */
  write(f, 4, SIZE_OF_HEADER + samples);         /* chunk size       */
  fputs("WAVE", f);                              /* file format      */
  fputs("fmt ", f);                              /* format chunk     */
  write(f, 4, 16);                               /* size of subchunk */
  write(f, 2, 1);                                /* format (1 = PCM) */
  write(f, 2, 1);                                /* # of channels    */
  write(f, 4, sample_rate);                      /* sample rate      */
  write(f, 4, sample_rate);                      /* byte rate        */
  write(f, 2, 1);                                /* block align      */
  write(f, 2, 8);                                /* bits per sample  */
  fputs("data", f);                              /* data chunk       */
  /* body */
  fwrite(data, 1, samples, f);                   /* actual audio     */

  fclose(f);
}
