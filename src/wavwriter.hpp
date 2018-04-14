// Copyright (c) C 2015, copied from https://github.com/porglezomp/wav-synth

#ifndef _WAV_SYNTH_WAVWRITER_H_INCLUDED
#define _WAV_SYNTH_WAVWRITER_H_INCLUDED

#include <stdint.h>
enum Format {
  PCM_U8,
  FLOAT_32,
};

void writewav(float *data, Format format, int num_channels, int samples, int sample_rate, const char* filename);
#endif
