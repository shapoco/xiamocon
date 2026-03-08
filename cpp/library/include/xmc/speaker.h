/**
 * @file speaker.h
 * @brief Speaker driver.
 */

#ifndef XMC_SPEAKER_H
#define XMC_SPEAKER_H

#include "xmc/hw/sdac.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Returns the supported audio sample formats for the speaker. This function is
 * a wrapper around xmc_sdac_get_supported_formats() since the speaker uses the
 * SDAC as its underlying audio output.
 * @return A bitmask of supported audio sample formats for the speaker.
 */
static inline xmc_audio_sample_format_t xmc_speaker_get_supported_formats(
    void) {
  return xmc_sdac_get_supported_formats();
}

/**
 * Initializes the speaker with the given audio format, sample rate, and
 * latency. This function will set up the underlying SDAC instance with the
 * specified configuration and prepare the speaker for use.
 * @param format The audio sample format to use for the speaker. This must be
 * one of the formats returned by xmc_speaker_get_supported_formats().
 * @param sample_rate_hz The sample rate in Hz to use for the speaker.
 * @param latency_samples The latency in samples for the speaker.
 * @param actual_rate_hz A pointer to a variable that will receive the actual
 * sample rate in Hz that the speaker will use.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
xmc_status_t xmc_speaker_init(xmc_audio_sample_format_t format,
                              uint32_t sample_rate_hz, uint32_t latency_samples,
                              float *actual_rate_hz);

/**
 * Deinitializes the speaker. This function will free any resources allocated
 * during initialization and reset the speaker hardware to a safe state.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
xmc_status_t xmc_speaker_deinit(void);

/**
 * Sets the muted state of the speaker. This function will mute or unmute the
 * speaker based on the provided boolean value.
 * @param muted A boolean value indicating whether to mute (true) or unmute
 * (false) the speaker.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
xmc_status_t xmc_speaker_set_muted(bool muted);

/**
 * Sets the audio source port for the speaker. This function will configure the
 * speaker to use the specified audio source port for audio data.
 * @param src A pointer to the audio source port to use for the speaker.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
xmc_status_t xmc_speaker_set_source_port(xmc_audio_source_port_t *src);

/**
 * Services the speaker. This function should be called periodically to ensure
 * that the speaker continues to receive audio data from the audio source and to
 * handle any necessary processing or buffering. This function is a wrapper
 * around xmc_sdac_service() since the speaker uses the SDAC as its underlying
 * audio output.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
xmc_status_t xmc_speaker_service(void);

#if defined(__cplusplus)
}
#endif

#endif
