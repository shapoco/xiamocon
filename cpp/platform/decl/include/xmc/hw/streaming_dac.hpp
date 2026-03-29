/**
 * @file sdac.hpp
 * @brief Streaming DAC.
 */

#ifndef XMC_HW_STREAMING_DAC_HPP
#define XMC_HW_STREAMING_DAC_HPP

#include "xmc/audio_common.hpp"
#include "xmc/xmc_common.hpp"

namespace xmc::audio {

/**
 * SDAC configuration structure. This structure defines the configuration
 * parameters for an SDAC instance, including the audio format and the
 * latency in samples.
 */
struct SdacConfig {
  StreamFormat format;
  uint32_t latencySamples;
};

/**
 * Get the supported audio sample formats for the SDAC. This function returns a
 * bitmask of the supported sample formats. The caller can check if a specific
 * sample format is supported by performing a bitwise AND operation with the
 * returned value and the desired sample format.
 */
SampleFormat sdacGetSupportedFormats(void);

/**
 * Get the preferred sampling rate for the SDAC. This is the sample rate
 * that the SDAC hardware is optimized for.
 * @return The preferred sampling rate in Hz.
 */
uint32_t getPreferredSamplingRate(void);

/**
 * Streaming DAC class. This class provides an interface for a streaming DAC,
 * which can be used to output audio data in real-time.
 */
class StreamingDac {
 public:
  const int pin;
  void *handle = nullptr;
  SourcePort source;
  StreamingDac(int pin);
  ~StreamingDac();

  /**
   * Start the SDAC with the given configuration. This will initialize the SDAC
   * hardware and prepare it for audio output.
   * @param cfg The configuration parameters for the SDAC.
   * @param actualRateHz Optional pointer to a float that will be set to the
   * actual sample rate of the SDAC.
   * @return XmcStatus indicating success or failure of the operation.
   */
  XmcStatus start(const SdacConfig &cfg, float *actualRateHz = nullptr);

  /**
   * Stop the SDAC. This will stop audio output and deinitialize the SDAC
   * hardware.
   * @return XmcStatus indicating success or failure of the operation.
   */
  XmcStatus stop();

  /**
   * Set the audio source for the SDAC. The SDAC will call the requestData
   * callback of the source port when it needs more audio data to output.
   * @param src The audio source port to set for the SDAC.
   * @return XmcStatus indicating success or failure of the operation.
   */
  XmcStatus setSource(SourcePort *src);

  /**
   * Service the SDAC. This should be called periodically to allow the SDAC to
   * process audio data and output it. The SDAC will call the requestData
   * callback of the source port as needed to get more audio data to output.
   * @return XmcStatus indicating success or failure of the operation.
   */
  XmcStatus service();
};

}  // namespace xmc::audio

#endif
