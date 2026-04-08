#include "xmc/speaker.hpp"
#include "xmc/audio_common.hpp"
#include "xmc/hw/pins.hpp"
#include "xmc/ioex.hpp"

namespace xmc::speaker {

audio::StreamingDac sdac(XMC_PIN_AUDIO_OUT);

XmcStatus init(audio::SampleFormat fmt, uint32_t rate, uint32_t latency,
               float *actualRateHz) {
  audio::StreamFormat streamFmt;
  audio::SdacConfig cfg;
  streamFmt.sampleFormat = fmt;
  streamFmt.rateHz = rate;
  cfg.format = streamFmt;
  cfg.latencySamples = latency;
  XMC_ERR_RET(ioex::write(ioex::Pin::SPEAKER_MUTE, 1));
  XMC_ERR_RET(ioex::setDir(ioex::Pin::SPEAKER_MUTE, true));
  XMC_ERR_RET(sdac.start(cfg, actualRateHz));
  return XMC_OK;
}

XmcStatus deinit(void) { return sdac.stop(); }

XmcStatus setMuted(bool muted) {
  return ioex::write(ioex::Pin::SPEAKER_MUTE, muted ? 1 : 0);
}

XmcStatus setSourcePort(audio::SourcePort *src) { return sdac.setSource(src); }

audio::StreamFormat getStreamFormat(void) { return sdac.getStreamFormat(); }

XmcStatus service(void) { return sdac.service(); }

}  // namespace xmc::speaker
