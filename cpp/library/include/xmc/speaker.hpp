/**
 * @file speaker.hpp
 * @brief Speaker driver.
 */

#ifndef XMC_SPEAKER_H
#define XMC_SPEAKER_H

#include "xmc/hw/streaming_dac.hpp"

namespace xmc::speaker {

/**
 * Returns the supported audio sample formats for the speaker. This function is
 * a wrapper around xmc::audio::sdacGetSupportedFormats() since the speaker uses
 * the SDAC as its underlying audio output.
 * @return A bitmask of supported audio sample formats for the speaker.
 */
static inline audio::SampleFormat xmc_speakerGetSupportedFormats(void) {
  return audio::sdacGetSupportedFormats();
}

/**
 * Initializes the speaker with the given audio format, sample rate, and
 * latency. This function will set up the underlying SDAC instance with the
 * specified configuration and prepare the speaker for use.
 * @param format The audio sample format to use for the speaker. This must be
 * one of the formats returned by xmc_speakerGetSupportedFormats().
 * @param rateHz The sample rate in Hz to use for the speaker.
 * @param latencySamples The latency in samples for the speaker.
 * @param actualRateHz A pointer to a variable that will receive the actual
 * sample rate in Hz that the speaker will use.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
XmcStatus init(audio::SampleFormat format, uint32_t rateHz,
               uint32_t latencySamples, float *actualRateHz);

/**
 * Deinitializes the speaker. This function will free any resources allocated
 * during initialization and reset the speaker hardware to a safe state.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
XmcStatus deinit(void);

/**
 * Sets the muted state of the speaker. This function will mute or unmute the
 * speaker based on the provided boolean value.
 * @param muted A boolean value indicating whether to mute (true) or unmute
 * (false) the speaker.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
XmcStatus setMuted(bool muted);

/**
 * Sets the audio source port for the speaker. This function will configure the
 * speaker to use the specified audio source port for audio data.
 * @param src A pointer to the audio source port to use for the speaker.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
XmcStatus setSourcePort(audio::SourcePort *src);

/**
 * Gets the current audio stream format of the speaker. This function will
 * return the audio format that the speaker is currently configured to use.
 * @return The current audio stream format of the speaker.
 */
audio::StreamFormat getStreamFormat(void);

/**
 * Services the speaker. This function should be called periodically to ensure
 * that the speaker continues to receive audio data from the audio source and to
 * handle any necessary processing or buffering. This function is a wrapper
 * around xmc_sdac_service() since the speaker uses the SDAC as its underlying
 * audio output.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
XmcStatus service(void);

}  // namespace xmc::speaker

#endif
