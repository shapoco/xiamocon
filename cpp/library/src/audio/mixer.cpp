#include "xmc/audio/mixer.hpp"

namespace xmc::audio {

static void xmc_mixerRequestData(void *buffer, uint32_t numSamples,
                                   void *context);

MixerClass::MixerClass(int numSources) : numSources(numSources) {
  sources = new SourcePort *[numSources];
  for (int i = 0; i < numSources; i++) {
    sources[i] = nullptr;
  }
  output.requestData = xmc_mixerRequestData;
  output.context = this;
}

MixerClass::~MixerClass() { delete[] sources; }

void MixerClass::render(int16_t *buffer, uint32_t numSamples) {
  for (int i = 0; i < numSources; i++) {
    if (sources[i]) {
      sources[i]->requestData(buffer, numSamples, sources[i]->context);
    }
  }
}

static void xmc_mixerRequestData(void *buffer, uint32_t numSamples,
                                   void *context) {
  ((MixerClass *)context)->render((int16_t *)buffer, numSamples);
}

}  // namespace xmc::audio
