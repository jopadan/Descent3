/*
 * Descent 3
 * Copyright (C) 2024 Parallax Software
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdint>
#include <cstdio>

#include "aencode.h"
#include "audio_encode.h"
#include "log.h"

int32_t aenc_ReadSamp(void *data) {
  FILE *f = (FILE *)data;
  int a, b;
  a = getc(f);
  if (a == EOF)
    return ReadSampleEof;

  b = getc(f);
  if (b == EOF)
    return ReadSampleEof;
  return (b << 8) | a;
}

bool aenc_Compress(const char *input_filename, const char *output_filename, const int *input_levels, const int *input_samples,
                   const int *input_rate, const int *input_channels, const float *input_factor,
                   const float *input_volscale) {
  FILE *in, *out;
  int32_t result;

  int levels = 0, samples_per_subband = 0;
  unsigned sample_rate = 0, channels = 0;
  float factor = 0, volume_scale = 0;
  int levels_set = 0, samples_per_subband_set = 0, sample_rate_set = 0, channels_set = 0, factor_set = 0,
      volume_scale_set = 0;

  in = fopen(input_filename, "rb");
  if (!in) {
    LOG_WARNING.printf("AENC: Unable to open %s for input.", input_filename);
    return false;
  }

  if (input_levels) {
    levels = *input_levels; // Levels (default 7 or for 2k total)
    levels_set = 1;
    LOG_WARNING_IF(levels < 0 || levels > 16) << "AENC: Warning: levels outside of the range 0 to 16";
  }

  if (input_samples) {
    samples_per_subband = *input_samples; // Samples per subband (default 16 or for 2k total)
    samples_per_subband_set = 1;
    LOG_WARNING_IF(samples_per_subband < 1 || samples_per_subband > 1024)
        << "AENC: Warning: samples per subband not in the range 1 to 1024";
  }

  if (input_rate) {
    sample_rate = *input_rate; // Sample rate (default 22K)
    sample_rate_set = 1;
    LOG_WARNING_IF(sample_rate != 11025 && sample_rate != 22050 && sample_rate != 44100)
        << "AENC: Warning: sample rate not 11025, 22050, or 44100";
  }

  if (input_channels) {
    channels = *input_channels;
    channels_set = 1;
    LOG_WARNING_IF(channels != 1 && channels != 2) << "AENC: Warning: /C channels not 1 or 2";
  }

  if (input_factor) {
    factor = *input_factor; // Factor of compression (default 4 for 22K, 8 for 44K)
    factor_set = 1;

    if (factor != 0.0f && factor < 1.0f)
      factor = 1.0f / factor;

    if (factor <= 0.0f) {
      LOG_WARNING << "AENC: Warning: compression factor <= 0.0";
      factor = 1.0f;
    }
  }

  if (input_volscale) {
    volume_scale = *input_volscale; // Volume scaling (slightly <= 1.0, default ,97)
    volume_scale_set = 1;
  }

  if (!levels_set && !samples_per_subband_set) {
    levels = 7;
    samples_per_subband = 16;
  } else if (!samples_per_subband_set) {
    samples_per_subband = 2048 / (1 << levels);

    if (samples_per_subband < 8)
      samples_per_subband = 8;
  } else if (!levels_set) {
    unsigned subbands = (2048 / samples_per_subband) >> 1;

    for (levels = 0; subbands; subbands >>= 1, ++levels) {
    }
  }

  if (!sample_rate_set)
    sample_rate = 22050;
  if (!channels_set)
    channels = 1;
  if (!factor_set)
    factor = sample_rate <= 22050 ? 4.0f : 8.0f;
  if (!volume_scale_set)
    volume_scale = .97f;

  out = fopen(output_filename, "wb");
  if (!out) {
    LOG_WARNING.printf("AENC: Unable to open %s for output.", output_filename);
    return false;
  }

  result = AudioEncode(aenc_ReadSamp, in, channels, sample_rate, volume_scale, out, levels, samples_per_subband,
                       1.0f / factor);

  fclose(out);
  fclose(in);

  return result > 0;
}
