/**
 * @file sdac.h
 * @brief Streaming DAC.
 */

#ifndef XMC_HW_AUDIO_DAC_H
#define XMC_HW_AUDIO_DAC_H

#include "xmc/audio_common.h"
#include "xmc/xmc_common.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * SDAC instance structure. This structure represents an instance of the SDAC
 * driver. It contains the hardware-specific data and the audio source port that
 * will be used to provide audio data to the SDAC.
 */
typedef struct {
  void *hw;
  int pin;
  xmc_audio_source_port_t source;
} xmc_sdac_inst_t;

/**
 * SDAC configuration structure. This structure defines the configuration
 * parameters for an SDAC instance, including the audio format and the
 * latency in samples.
 */
typedef struct {
  xmc_audio_format_t format;
  uint32_t latency_samples;
} xmc_sdac_config_t;

/**
 * Returns the supported audio sample formats for the SDAC.
 * @return A bitmask of supported audio sample formats.
 */
xmc_audio_sample_format_t xmc_sdac_get_supported_formats(void);

/**
 * Initializes the SDAC instance with the given configuration. This function
 * will set up the SDAC hardware, allocate necessary resources, and prepare the
 * instance for use.
 * @param inst The SDAC instance to initialize.
 * @param pin The GPIO pin to use for the SDAC output.
 * @param cfg The configuration parameters for the SDAC instance.
 * @param actual_rate_hz A pointer to a variable that will receive the actual
 * sample rate in Hz that the SDAC will use. This may be different from the
 * requested sample rate in the configuration due to hardware limitations.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
xmc_status_t xmc_sdac_init(xmc_sdac_inst_t *inst, int pin,
                           const xmc_sdac_config_t *cfg, float *actual_rate_hz);

/**
 * Deinitializes the SDAC instance. This function will free any resources
 * allocated during initialization and reset the SDAC hardware to a safe state.
 * @param inst The SDAC instance to deinitialize.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
xmc_status_t xmc_sdac_deinit(xmc_sdac_inst_t *inst);

/**
 * Sets the audio source for the SDAC instance. The audio source is defined by
 * an xmc_audio_source_port_t structure, which contains a callback function that
 * will be called when audio data is requested from the SDAC. The callback
 * function will receive a buffer to write the audio samples to, the number of
 * samples to write, and a context pointer that can be used to pass user-defined
 * data to the callback function.
 * @param inst The SDAC instance to set the audio source for.
 * @param src A pointer to the audio source port to use as the audio source for
 * the SDAC instance.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
xmc_status_t xmc_sdac_set_source(xmc_sdac_inst_t *inst,
                                 xmc_audio_source_port_t *src);

/**
 * Services the SDAC instance. This function should be called periodically to
 * ensure that the SDAC continues to receive audio data from the audio source
 * and to handle any necessary processing or buffering.
 * @param inst The SDAC instance to service.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
xmc_status_t xmc_sdac_service(xmc_sdac_inst_t *inst);

#if defined(__cplusplus)
}
#endif

#endif
