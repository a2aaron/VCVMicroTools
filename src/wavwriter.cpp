// Copyright (c) C 2015, copied from https://github.com/porglezomp/wav-synth

#include "wavwriter.hpp"

#include <cassert>
#include <cstdio>

#define SIZE_OF_HEADER 36

void write(FILE *f, int size, int32_t arg) {
  uint8_t x;
  int16_t y;
  int32_t z;

  switch (size) {
  case 1:
    x = (uint8_t)arg;
    fwrite(&x, size, 1, f);
    break;
  case 2:
    y = (int16_t)arg;
    fwrite(&y, size, 1, f);
    break;
  case 4:
    z = (int32_t)arg;
    fwrite(&z, size, 1, f);
    break;
  }
}

const char *toString(SampleFmt format) {
  switch (format) {
  case SampleFmt::PCM_U8:
    return "8 bit unsigned";
  case SampleFmt::PCM_S16:
    return "16 bit signed";
  case SampleFmt::FLOAT_32:
    return "32 bit float";
  default:
    assert(not"an expected format");
  }
}

void writewav(uint8_t *data, SampleFmt format, int num_channels, int samples,
              int sample_rate, const char *filename) {
  // Note: This is "write bytes" as to avoid Windows from sticking `0d = \r`
  // before every `0a = \n` (CLRF vs LF line ending nonsense).
  FILE *f = fopen(filename, "wb");

  int sample_bytes = 0;
  int wav_format = 0;
  switch (format) {
  case PCM_U8:
    sample_bytes = 1, wav_format = 1;
    break;
  case PCM_S16:
    sample_bytes = 2, wav_format = 1;
    break;
  case FLOAT_32:
    sample_bytes = 4, wav_format = 3;
    break;
  default:
    assert(not"an expected format");
  }
  int total_bytes = num_channels * sample_bytes * samples;
  int block_align = num_channels * sample_bytes;

  /* header*/
  fputs("RIFF", f);                          /* main chunk       */
  write(f, 4, SIZE_OF_HEADER + total_bytes); /* chunk size       */
  fputs("WAVE", f);                          /* file format      */
  fputs("fmt ", f);                          /* format chunk     */
  write(f, 4, 16);                           /* size of subchunk */
  write(f, 2, wav_format);                   /* format           */
  write(f, 2, num_channels);                 /* # of channels    */
  write(f, 4, sample_rate);                  /* sample rate      */
  write(f, 4, block_align * sample_rate);    /* byte rate        */
  write(f, 2, block_align);                  /* block align      */
  write(f, 2, 8 * sample_bytes);             /* bits per sample  */
  fputs("data", f);                          /* data chunk       */

  /* body */
  fwrite(data, 1, total_bytes, f); /* actual audio     */

  fclose(f);
}
