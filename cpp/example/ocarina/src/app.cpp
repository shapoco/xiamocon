#include <xiamocon.hpp>

using namespace xmc;
using namespace xmc::audio;
using namespace xmc::input;

static constexpr int NUM_TONES = 4;
static constexpr int NUM_KEYS = 8;
static constexpr uint32_t SAMPLE_RATE_HZ = 44100;

static constexpr PixelFormat DISPLAY_FORMAT = PixelFormat::RGB444;
FrameBuffer frameBuffer(DISPLAY_FORMAT, true);

static Mixer mixer(NUM_TONES);
static Tone tones[NUM_TONES];

static Waveform waveform = Waveform::SQUARE;

static int nextToneIndex = 0;
static Button keys[] = {
    Button::DOWN, Button::LEFT, Button::UP, Button::RIGHT,
    Button::X,    Button::Y,    Button::A,  Button::B,
};
static int keyToTone[] = {-1, -1, -1, -1, -1, -1, -1, -1};
static int keyToNote[] = {-5, -3, -1, 0, 2, 4, 5, 7};

bool dispUpdate = false;

AppConfig xmc::appGetConfig() {
  AppConfig cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = DISPLAY_FORMAT;
  cfg.speakerSampleFormat = SampleFormat::LINEAR_PCM_S16_MONO;
  cfg.speakerSampleRateHz = SAMPLE_RATE_HZ;
  cfg.speakerLatencySamples = 512;
  return cfg;
}

void xmc::appSetup() {
  for (int i = 0; i < NUM_TONES; i++) {
    tones[i].init(SAMPLE_RATE_HZ);
    mixer.setSource(i, tones[i].getOutputPort());
  }
  for (int i = 0; i < NUM_KEYS; i++) {
    keyToTone[i] = -1;
  }

  speaker::setSourcePort(mixer.getOutputPort());
  gpio::setDir(XMC_PIN_GPIO_0, true);
  speaker::setMuted(false);

  dispUpdate = true;
}

void xmc::appLoop() {
  if (input::wasPressed(input::Button::FUNC)) {
    int n = static_cast<int>(Waveform::NUM_WAVEFORMS);
    waveform = static_cast<Waveform>((static_cast<int>(waveform) + 1) % n);
  }

  for (int ikey = 0; ikey < NUM_KEYS; ikey++) {
    if (input::wasPressed(keys[ikey])) {
      if (keyToTone[ikey] >= 0) {
        // this key is already playing a tone, so stop it first
        tones[keyToTone[ikey]].noteOff();
      }

      keyToTone[ikey] = nextToneIndex;
      nextToneIndex = (nextToneIndex + 1) % NUM_TONES;

      tones[keyToTone[ikey]].setWaveform(waveform);
      tones[keyToTone[ikey]].setVelocity(64);
      tones[keyToTone[ikey]].setEnvelope(0, 1000, 192, 500);
      tones[keyToTone[ikey]].noteOn(64 + keyToNote[ikey]);

      dispUpdate = true;
    }
    if (input::wasReleased(keys[ikey])) {
      if (keyToTone[ikey] >= 0) {
        tones[keyToTone[ikey]].noteOff();
        keyToTone[ikey] = -1;
      }
      dispUpdate = true;
    }
  }

  if (dispUpdate) {
    dispUpdate = false;

    frameBuffer.beginRender();

    Graphics2D gfx = frameBuffer.createGraphics();

    gfx->clear(0);
    for (int i = 0; i < NUM_KEYS; i++) {
      int x = display::WIDTH * i / NUM_KEYS + 2;
      int y = display::HEIGHT / 4;
      int w = display::WIDTH / NUM_KEYS - 4;
      int h = display::HEIGHT / 2;
      gfx->fillRect(x, y, w, h, (keyToTone[i] < 0) ? 0xFFF : 0x0D6);
    }

    static const int blackKeyX[] = {
        display::HEIGHT * -1 / 8 + display::HEIGHT * 1 / 14,
        display::HEIGHT * -1 / 8 + display::HEIGHT * 3 / 14,
        display::HEIGHT * -1 / 8 + display::HEIGHT * 5 / 14,
        display::HEIGHT * 3 / 8 + display::HEIGHT * 1 / 14,
        display::HEIGHT * 3 / 8 + display::HEIGHT * 3 / 14,
        display::HEIGHT * 6 / 8 + display::HEIGHT * 1 / 14,
        display::HEIGHT * 6 / 8 + display::HEIGHT * 3 / 14,
    };
    static constexpr int NUM_BLACK_KEYS =
        (int)(sizeof(blackKeyX) / sizeof(blackKeyX[0]));
    for (int i = 0; i < NUM_BLACK_KEYS; i++) {
      int x = blackKeyX[i];
      int y = display::HEIGHT / 4;
      int w = display::WIDTH / 12;
      int h = display::HEIGHT * 3 / 10;
      gfx->fillRect(x, y, w, h, 0x000);
    }

    frameBuffer.endRender();
  }
}
