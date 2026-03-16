#include "xmc/app.h"
#include "xmc/audio/tone.hpp"
#include "xmc/display.h"
#include "xmc/gfx.hpp"
#include "xmc/hw/timer.h"
#include "xmc/input.h"
#include "xmc/speaker.h"
#include "xmc/battery.h"
#include "xmc/font/ShapoSansP_s08c07.h"

#include <stdint.h>
#include <stdio.h>


static constexpr uint32_t SAMPLE_RATE_HZ = 22050;

int r_counter = 0, g_counter = 32767, b_counter = 65535;
float x = XMC_DISPLAY_WIDTH / 2, y = XMC_DISPLAY_HEIGHT / 2;
float dx = 1.0f, dy = 1.11f;

xmc::Sprite444 frame_buffer(XMC_DISPLAY_WIDTH, XMC_DISPLAY_HEIGHT);
xmc::Tone tone;

xmc::Waveform waveform = xmc::Waveform::SQUARE;

xmc_app_config_t xmc_app_get_config() {
  xmc_app_config_t config = xmc_get_default_app_config();
  config.speaker_sample_rate_hz = SAMPLE_RATE_HZ;
  return config;
}

void xmc_app_setup() {
  frame_buffer.clear(0);
  xmc_sleep_ms(1);
  tone.init(SAMPLE_RATE_HZ);
  xmc_speaker_set_source_port(tone.get_output_port());
  xmc_speaker_set_muted(false);
}

void xmc_app_loop() {
  uint16_t battery_mv = xmc_battery_get_voltage_mv();
  char buf[32];
  snprintf(buf, sizeof(buf), "Battery: %d mV", battery_mv);


  xmc_button_t buttons = xmc_input_get_state();
  if (buttons & XMC_BUTTON_LEFT) {
    dx -= 0.1f;
  }
  if (buttons & XMC_BUTTON_RIGHT) {
    dx += 0.1f;
  }
  if (buttons & XMC_BUTTON_UP) {
    dy -= 0.1f;
  }
  if (buttons & XMC_BUTTON_DOWN) {
    dy += 0.1f;
  }

  if (xmc_input_was_pressed(XMC_BUTTON_Y)) {
    int n = static_cast<int>(xmc::Waveform::NUM_WAVEFORMS);
    waveform = static_cast<xmc::Waveform>((static_cast<int>(waveform) + 1) % n);
  }

  if (xmc_input_was_pressed(XMC_BUTTON_A)) {
    // tone.set_waveform(xmc::Waveform::SQUARE);
    tone.set_waveform(waveform);
    // tone.set_waveform(xmc::Waveform::TRIANGLE);
    tone.set_velocity(127);
    tone.set_envelope(0, 2000, 0, 500);
    // tone.set_sweep(1600, 10);
    tone.note_on(64 + 12);
    dx += 1;
  }
  if (xmc_input_was_released(XMC_BUTTON_A)) {
    tone.note_off();
  }

  dy += 0.1f;

  dx *= 0.999f;
  dy *= 0.999f;

  x += dx;
  y += dy;
  if (x < 0 || x >= XMC_DISPLAY_WIDTH) {
    dx = -dx;
    x += dx;
  }
  if (y < 0 || y >= XMC_DISPLAY_HEIGHT) {
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
  frame_buffer.complete_transfer();

  // fill box
  frame_buffer.fill_rect((int)x - 32, (int)y - 32, 64, 64, color);

  frame_buffer.set_font(&ShapoSansP_s08c07);
  frame_buffer.fill_rect(0, 0, 128, 8, 0x000);
  frame_buffer.set_cursor(0, 7);
  frame_buffer.set_text_color(0xFFF);
  frame_buffer.draw_string(buf);

  // start transferring the current frame to the display. This will return
  // immediately and the transfer will happen in the background.
  frame_buffer.start_transfer_to_display(0, 0);

  xmc_sleep_ms(10);
}
