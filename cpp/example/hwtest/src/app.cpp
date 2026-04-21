#include <xiamocon.hpp>

#include <stdint.h>
#include <stdio.h>

using namespace xmc;
using namespace audio;

static constexpr PixelFormat DISPLAY_FORMAT = PixelFormat::RGB444;
static constexpr uint32_t SAMPLE_RATE_HZ = 24000;

int rCounter = 0, gCounter = 32767, bCounter = 65535;
float x = display::WIDTH / 2, y = display::HEIGHT / 2;
float dx = 1.0f, dy = 1.11f;

FrameBuffer frameBuffer = createFrameBuffer(DISPLAY_FORMAT, true);
Tone tone;

Waveform waveform = Waveform::SQUARE;

AppConfig xmc::appGetConfig() {
  AppConfig cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = DISPLAY_FORMAT;
  cfg.speakerSampleFormat = SampleFormat::LINEAR_PCM_S16_MONO;
  cfg.speakerSampleRateHz = SAMPLE_RATE_HZ;
  return cfg;
}

void xmc::appSetup() {
  frameBuffer->enableFlag(FrameBufferFlags::SHOW_DEBUG_INFO);
  tone.init(SAMPLE_RATE_HZ);
  speaker::setSourcePort(tone.getOutputPort());
  speaker::setMuted(false);
}

void xmc::appLoop() {
  uint16_t battery_mv = battery::getVoltageMv();
  char buf[32];
  snprintf(buf, sizeof(buf), "Battery: %d mV", battery_mv);

  input::Button buttons = input::getState();
  if (hasFlag(buttons, input::Button::LEFT)) {
    dx -= 0.1f;
  }
  if (hasFlag(buttons, input::Button::RIGHT)) {
    dx += 0.1f;
  }
  if (hasFlag(buttons, input::Button::UP)) {
    dy -= 0.1f;
  }
  if (hasFlag(buttons, input::Button::DOWN)) {
    dy += 0.1f;
  }

  if (input::wasPressed(input::Button::Y)) {
    int n = static_cast<int>(Waveform::NUM_WAVEFORMS);
    waveform = static_cast<Waveform>((static_cast<int>(waveform) + 1) % n);
  }

  if (input::wasPressed(input::Button::A)) {
    // tone.setWaveform(Waveform::SQUARE);
    tone.setWaveform(waveform);
    // tone.setWaveform(Waveform::TRIANGLE);
    tone.setVelocity(127);
    tone.setEnvelope(0, 2000, 0, 500);
    // tone.setSweep(1600, 10);
    tone.noteOn(64 + 12);
    dx += 1;
  }
  if (input::wasReleased(input::Button::A)) {
    tone.noteOff();
  }

  dy += 0.1f;

  dx *= 0.999f;
  dy *= 0.999f;

  x += dx;
  y += dy;
  if (x < 0 || x >= display::WIDTH) {
    dx = -dx;
    x += dx;
  }
  if (y < 0 || y >= display::HEIGHT) {
    dy = -dy;
    y += dy;
  }

  rCounter = (rCounter + 1100) % 65536;
  gCounter = (gCounter + 1200) % 65536;
  bCounter = (bCounter + 1300) % 65536;
  int r = rCounter >> 11;
  int g = gCounter >> 11;
  int b = bCounter >> 11;
  if (r >= 16) r = 31 - r;
  if (g >= 16) g = 31 - g;
  if (b >= 16) b = 31 - b;
  uint32_t color = ((r << 8) | (g << 4) | b);

  frameBuffer->beginRender();

  Graphics2D gfx = frameBuffer->createGraphics();
  gfx->clear(0);

  // fill box
  gfx->fillRect((int)x - 32, (int)y - 32, 64, 64, color);

  frameBuffer->endRender();

  sleepMs(10);
}
