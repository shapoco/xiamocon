#include "xiamocon.hpp"

using namespace xmc;
using namespace xmc::audio;
using namespace xmc::input;

static constexpr int NUM_TONES = 4;
static constexpr int NUM_KEYS = 8;
static constexpr uint32_t SAMPLE_RATE_HZ = 44100;

static Sprite screen = createSprite444(display::WIDTH, display::HEIGHT);

static Mixer mixer(NUM_TONES);
static Tone tones[NUM_TONES];

static Waveform waveform = Waveform::SQUARE;

static int next_tone_index = 0;
static Button keys[] = {
    Button::DOWN, Button::LEFT, Button::UP, Button::RIGHT,
    Button::X,    Button::Y,    Button::A,  Button::B,
};
static int key_to_tone[] = {-1, -1, -1, -1, -1, -1, -1, -1};
static int key_to_note[] = {-5, -3, -1, 0, 2, 4, 5, 7};

bool disp_update = false;

AppConfig xmc::appGetConfig() {
  AppConfig cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = PixelFormat::RGB444;
  cfg.speakerSampleFormat = SampleFormat::LINEAR_PCM_S16_MONO;
  cfg.speakerSampleRateHz = SAMPLE_RATE_HZ;
  cfg.speakerLatencySamples = 512;
  return cfg;
}

void xmc::appSetup() {
  screen->clear(0);
  for (int i = 0; i < NUM_TONES; i++) {
    tones[i].init(SAMPLE_RATE_HZ);
    mixer.setSource(i, tones[i].getOutputPort());
  }
  for (int i = 0; i < NUM_KEYS; i++) {
    key_to_tone[i] = -1;
  }

  speaker::setSourcePort(mixer.getOutputPort());
  gpio::setDir(XMC_PIN_GPIO_0, true);
  speaker::setMuted(false);

  disp_update = true;
}

void xmc::appLoop() {
  if (input::wasPressed(input::Button::FUNC)) {
    int n = static_cast<int>(Waveform::NUM_WAVEFORMS);
    waveform = static_cast<Waveform>((static_cast<int>(waveform) + 1) % n);
  }

  for (int ikey = 0; ikey < NUM_KEYS; ikey++) {
    if (input::wasPressed(keys[ikey])) {
      if (key_to_tone[ikey] >= 0) {
        // this key is already playing a tone, so stop it first
        tones[key_to_tone[ikey]].noteOff();
      }

      key_to_tone[ikey] = next_tone_index;
      next_tone_index = (next_tone_index + 1) % NUM_TONES;

      tones[key_to_tone[ikey]].setWaveform(waveform);
      tones[key_to_tone[ikey]].setVelocity(64);
      tones[key_to_tone[ikey]].setEnvelope(0, 1000, 192, 500);
      tones[key_to_tone[ikey]].noteOn(64 + key_to_note[ikey]);

      disp_update = true;
    }
    if (input::wasReleased(keys[ikey])) {
      if (key_to_tone[ikey] >= 0) {
        tones[key_to_tone[ikey]].noteOff();
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
    screen->completeTransfer();

    screen->clear(0);
    for (int i = 0; i < NUM_KEYS; i++) {
      int x = display::WIDTH * i / NUM_KEYS + 2;
      int y = display::HEIGHT / 4;
      int w = display::WIDTH / NUM_KEYS - 4;
      int h = display::HEIGHT / 2;
      screen->fillRect(x, y, w, h, (key_to_tone[i] < 0) ? 0xFFF : 0x0D6);
    }

    static const int black_key_x[] = {
        display::HEIGHT * -1 / 8 + display::HEIGHT * 1 / 14,
        display::HEIGHT * -1 / 8 + display::HEIGHT * 3 / 14,
        display::HEIGHT * -1 / 8 + display::HEIGHT * 5 / 14,
        display::HEIGHT * 3 / 8 + display::HEIGHT * 1 / 14,
        display::HEIGHT * 3 / 8 + display::HEIGHT * 3 / 14,
        display::HEIGHT * 6 / 8 + display::HEIGHT * 1 / 14,
        display::HEIGHT * 6 / 8 + display::HEIGHT * 3 / 14,
    };
    for (int i = 0; i < sizeof(black_key_x) / sizeof(black_key_x[0]); i++) {
      int x = black_key_x[i];
      int y = display::HEIGHT / 4;
      int w = display::WIDTH / 12;
      int h = display::HEIGHT * 3 / 10;
      screen->fillRect(x, y, w, h, 0x000);
    }

    // start transferring the current frame to the display. This will return
    // immediately and the transfer will happen in the background.
    screen->startTransferToDisplay(0, 0);
  }
}
