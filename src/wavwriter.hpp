// Copyright (c) C 2015, copied from https://github.com/porglezomp/wav-synth

#ifndef WAV_SYNTH_WAVWRITER_H_INCLUDED
#define WAV_SYNTH_WAVWRITER_H_INCLUDED

#include <cstdint>

enum SampleFmt {
  PCM_U8,
  PCM_S16,
  FLOAT_32,
};

const char *toString(SampleFmt format);

void writewav(uint8_t *data, SampleFmt format, int num_channels, int samples,
              int sample_rate, const char *filename);

#endif
