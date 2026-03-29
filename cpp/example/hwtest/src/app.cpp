#include "xiamocon.hpp"

#include "xmc/font/ShapoSansP_s08c07.h"

#include <stdint.h>
#include <stdio.h>

using namespace xmc;
using namespace audio;

static constexpr uint32_t SAMPLE_RATE_HZ = 24000;

int r_counter = 0, g_counter = 32767, b_counter = 65535;
float x = display::WIDTH / 2, y = display::HEIGHT / 2;
float dx = 1.0f, dy = 1.11f;

Sprite frame_buffer = createSprite444(display::WIDTH, display::HEIGHT);
Tone tone;

Waveform waveform = Waveform::SQUARE;

AppConfig xmc::appGetConfig() {
  AppConfig cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = display::InterfaceFormat::RGB444;
  cfg.speakerSampleFormat = SampleFormat::LINEAR_PCM_S16_MONO;
  cfg.speakerSampleRateHz = SAMPLE_RATE_HZ;
  return cfg;
}

void xmc::appSetup() {
  frame_buffer->clear(0);
  sleepMs(1);
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

  r_counter = (r_counter + 1100) % 65536;
  g_counter = (g_counter + 1200) % 65536;
  b_counter = (b_counter + 1300) % 65536;
  int r = r_counter >> 11;
  int g = g_counter >> 11;
  int b = b_counter >> 11;
  if (r >= 16) r = 31 - r;
  if (g >= 16) g = 31 - g;
  if (b >= 16) b = 31 - b;
  uint32_t color = ((r << 8) | (g << 4) | b);

  // complete the previous frame's transfer if it's still in progress, then fill
  // the frame buffer with the new frame's content. In this case, we just draw a
  // moving box, but you can draw anything you want here.
  frame_buffer->completeTransfer();

  // fill box
  frame_buffer->fillRect((int)x - 32, (int)y - 32, 64, 64, color);

  xmc::appDrawDebugInfo(frame_buffer);
  xmc::appDrawStatusBar(frame_buffer);

  // start transferring the current frame to the display. This will return
  // immediately and the transfer will happen in the background.
  frame_buffer->startTransferToDisplay(0, 0);

  sleepMs(10);
}
