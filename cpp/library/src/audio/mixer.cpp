#include "xmc/audio/mixer.hpp"

namespace xmc {

static void xmc_mixer_request_data(void *buffer, uint32_t num_samples,
                                   void *context);

Mixer::Mixer(int num_sources) {
  sources = new xmc_audio_source_port_t *[num_sources];
  for (int i = 0; i < num_sources; i++) {
    sources[i] = nullptr;
  }
  output.request_data = xmc_mixer_request_data;
  output.context = this;
}

Mixer::~Mixer() { delete[] sources; }

void Mixer::render(int16_t *buffer, uint32_t num_samples) {
  for (int i = 0; sources[i] != nullptr; i++) {
    sources[i]->request_data(buffer, num_samples, sources[i]->context);
  }
}

static void xmc_mixer_request_data(void *buffer, uint32_t num_samples,
                                   void *context) {
  ((Mixer *)context)->render((int16_t *)buffer, num_samples);
}

}  // namespace xmc
