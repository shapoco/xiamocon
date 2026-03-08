#include "xmc/app.h"
#include "xmc/audio.hpp"
#include "xmc/display.h"
#include "xmc/gfx.hpp"
#include "xmc/hw/timer.h"
#include "xmc/input.h"
#include "xmc/speaker.h"

#include "xmc/hw/gpio.h"
#include "xmc/hw/pins.h"

#include <stdint.h>

static constexpr int NUM_TONES = 4;
static constexpr int NUM_KEYS = 8;
static constexpr uint32_t SAMPLE_RATE_HZ = 44100;

static xmc::Sprite444 frame_buffer(XMC_DISPLAY_WIDTH, XMC_DISPLAY_HEIGHT);

static xmc::Mixer mixer(NUM_TONES);
static xmc::Tone tones[NUM_TONES];

static xmc::Waveform waveform = xmc::Waveform::SQUARE;

static int next_tone_index = 0;
static xmc_button_t keys[] = {
    XMC_BUTTON_DOWN, XMC_BUTTON_LEFT, XMC_BUTTON_UP, XMC_BUTTON_RIGHT,
    XMC_BUTTON_X,    XMC_BUTTON_Y,    XMC_BUTTON_A,  XMC_BUTTON_B,
};
static int key_to_tone[] = {-1, -1, -1, -1, -1, -1, -1, -1};
static int key_to_note[] = {-5, -3, -1, 0, 2, 4, 5, 7};

bool disp_update = false;

void xmc_app_setup() {
  frame_buffer.clear(0);
  xmc_speaker_init(XMC_SAMPLE_LINEAR_PCM_S16_MONO, SAMPLE_RATE_HZ, 1024,
                   nullptr);
  for (int i = 0; i < NUM_TONES; i++) {
    tones[i].init(SAMPLE_RATE_HZ);
    mixer.set_source(i, tones[i].get_output_port());
  }
  for (int i = 0; i < NUM_KEYS; i++) {
    key_to_tone[i] = -1;
  }

  xmc_speaker_set_source_port(mixer.get_output_port());
  xmc_gpio_set_dir(XMC_PIN_GPIO_0, true);
  xmc_speaker_set_muted(false);

  disp_update = true;
}

void xmc_app_loop() {
  xmc_input_service();
  xmc_speaker_service();

  if (xmc_input_was_pressed(XMC_BUTTON_FUNC)) {
    int n = static_cast<int>(xmc::Waveform::NUM_WAVEFORMS);
    waveform = static_cast<xmc::Waveform>((static_cast<int>(waveform) + 1) % n);
  }

  for (int ikey = 0; ikey < NUM_KEYS; ikey++) {
    if (xmc_input_was_pressed(keys[ikey])) {
      if (key_to_tone[ikey] >= 0) {
        // this key is already playing a tone, so stop it first
        tones[key_to_tone[ikey]].note_off();
      }

      key_to_tone[ikey] = next_tone_index;
      next_tone_index = (next_tone_index + 1) % NUM_TONES;

      tones[key_to_tone[ikey]].set_waveform(waveform);
      tones[key_to_tone[ikey]].set_velocity(64);
      tones[key_to_tone[ikey]].set_envelope(0, 1000, 192, 500);
      tones[key_to_tone[ikey]].note_on(64 + key_to_note[ikey]);

      disp_update = true;
    }
    if (xmc_input_was_released(keys[ikey])) {
      if (key_to_tone[ikey] >= 0) {
        tones[key_to_tone[ikey]].note_off();
        key_to_tone[ikey] = -1;
      }
      disp_update = true;
    }
  }

  if (disp_update) {
    disp_update = false;
    
    // complete the previous frame's transfer if it's still in progress, then
    // fill the frame buffer with the new frame's content. In this case, we just
    // draw a moving box, but you can draw anything you want here.
    frame_buffer.complete_transfer();

    frame_buffer.clear(0);
    for (int i = 0; i < NUM_KEYS; i++) {
      int x = XMC_DISPLAY_WIDTH * i / NUM_KEYS + 2;
      int y = XMC_DISPLAY_HEIGHT / 4;
      int w = XMC_DISPLAY_WIDTH / NUM_KEYS - 4;
      int h = XMC_DISPLAY_HEIGHT / 2;
      frame_buffer.fill_rect(x, y, w, h, (key_to_tone[i] < 0) ? 0xFFF : 0x0D6);
    }

    static const int black_key_x[] = {
        XMC_DISPLAY_HEIGHT * -1 / 8 + XMC_DISPLAY_HEIGHT * 1 / 14,
        XMC_DISPLAY_HEIGHT * -1 / 8 + XMC_DISPLAY_HEIGHT * 3 / 14,
        XMC_DISPLAY_HEIGHT * -1 / 8 + XMC_DISPLAY_HEIGHT * 5 / 14,
        XMC_DISPLAY_HEIGHT * 3 / 8 + XMC_DISPLAY_HEIGHT * 1 / 14,
        XMC_DISPLAY_HEIGHT * 3 / 8 + XMC_DISPLAY_HEIGHT * 3 / 14,
        XMC_DISPLAY_HEIGHT * 6 / 8 + XMC_DISPLAY_HEIGHT * 1 / 14,
        XMC_DISPLAY_HEIGHT * 6 / 8 + XMC_DISPLAY_HEIGHT * 3 / 14,
    };
    for (int i = 0; i < sizeof(black_key_x) / sizeof(black_key_x[0]); i++) {
      int x = black_key_x[i];
      int y = XMC_DISPLAY_HEIGHT / 4;
      int w = XMC_DISPLAY_WIDTH / 12;
      int h = XMC_DISPLAY_HEIGHT * 3 / 10;
      frame_buffer.fill_rect(x, y, w, h, 0x000);
    }

    // start transferring the current frame to the display. This will return
    // immediately and the transfer will happen in the background.
    frame_buffer.start_transfer_to_display(0, 0);
  }
}
