/**
 * @file audio_common.h
 * @brief Common definitions for audio components in the XMC library.
 */

#ifndef XMC_AUDIO_COMMON_H
#define XMC_AUDIO_COMMON_H

#include "xmc/xmc_common.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Audio sample format. This defines the format of the audio samples that are
 * used in the audio system. The sample format determines how the audio data is
 * represented in memory and how it should be interpreted when rendering audio.
 */
typedef enum {
  /** 8-bit unsigned linear PCM, mono */
  XMC_SAMPLE_LINEAR_PCM_U8_MONO = (1 << 0),
  /** 16-bit signed linear PCM, mono */
  XMC_SAMPLE_LINEAR_PCM_S16_MONO = (1 << 1),
} xmc_audio_sample_format_t;

/**
 * Audio format. This structure defines the format of the audio data, including
 * the sample format and the sample rate.
 */
typedef struct {
  xmc_audio_sample_format_t sample_format;
  uint32_t sample_rate_hz;
} xmc_audio_format_t;

/**
 * Audio stream request callback. This is the type of the callback function that
 * will be called when audio data is requested from an audio source. The
 * callback function will receive a buffer to write the audio samples to, the
 * number of samples to write, and a context pointer that can be used to pass
 * user-defined data to the callback function.
 */
typedef void (*xmc_stream_request_t)(void *buffer, uint32_t num_samples,
                                     void *context);

/**
 * Audio source port. This structure defines a port that can be used as an audio
 * source. It contains a callback function that will be called when audio data
 * is requested, and a context pointer that can be used to pass user-defined
 * data to the callback function.
 */
typedef struct {
  xmc_stream_request_t request_data;
  void *context;
} xmc_audio_source_port_t;

/**
 * Returns the number of bytes per sample for the given audio sample format.
 * This function can be used to determine how much memory is needed to store a
 * certain number of audio samples in a given format.
 * @param format The audio sample format to get the bytes per sample for.
 * @return The number of bytes per sample for the given audio sample format, or
 * 0 if the format is unknown or unsupported.
 */
static inline int xmc_audio_get_bytes_per_sample(
    xmc_audio_sample_format_t format) {
  switch (format) {
    case XMC_SAMPLE_LINEAR_PCM_U8_MONO: return 1;
    case XMC_SAMPLE_LINEAR_PCM_S16_MONO: return 2;
    default: return 0;
  }
}

#if defined(__cplusplus)
}
#endif

#endif
