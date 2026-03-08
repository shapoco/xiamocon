/**
 * @file mixer.hpp
 * @brief A simple audio mixer that can mix multiple audio sources together.
 */

#ifndef XMC_AUDIO_MIXER_HPP
#define XMC_AUDIO_MIXER_HPP

#include "xmc/audio_common.h"

namespace xmc {
class Mixer {
 private:
  xmc_audio_source_port_t **sources;
  xmc_audio_source_port_t output;

 public:
  /**
   * @param num_sources The number of audio sources that can be mixed together.
   * This
   */
  Mixer(int num_sources);

  ~Mixer();

  /**
   * Sets the audio source for a given index. The mixer will mix together all
   * sources that are set. The index must be between 0 and the number of sources
   * specified in the constructor. The source must remain valid as long as it is
   * set on the mixer.
   */
  inline void set_source(int index, xmc_audio_source_port_t *source) {
    sources[index] = source;
  }

  /**
   * Renders the mixed audio data into the given buffer. This is called by the
   * audio system when it needs more audio data. The buffer is an array of
   * 16-bit signed integers, and num_samples is the number of samples to render.
   * The mixer should fill the buffer with the mixed audio data from all
   * sources. The mixer should not write more than num_samples samples to the
   * buffer.
   */
  void render(int16_t *buffer, uint32_t num_samples);

  /**
   * Returns the output audio source port of the mixer. The mixer will call the
   * request_data callback of the output port when it needs more audio data.
   */
  inline xmc_audio_source_port_t *get_output_port() { return &output; }
};

}  // namespace xmc

#endif
