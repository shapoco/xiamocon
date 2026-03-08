#ifndef XMC_AUDIO_MIXER_HPP
#define XMC_AUDIO_MIXER_HPP

#include "xmc/audio_common.h"

namespace xmc {
class Mixer {
 private:
  xmc_audio_source_port_t **sources;
  xmc_audio_source_port_t output;

 public:
  Mixer(int num_sources);

  ~Mixer();

  inline void set_source(int index, xmc_audio_source_port_t *source) {
    sources[index] = source;
  }

  void render(int16_t *buffer, uint32_t num_samples);

  inline xmc_audio_source_port_t *get_output_port() { return &output; }
};

}  // namespace xmc

#endif
