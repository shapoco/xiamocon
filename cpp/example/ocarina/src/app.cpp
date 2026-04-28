#include <xiamocon.hpp>

using namespace xmc;
using namespace xmc::audio;
using namespace xmc::input;

constexpr int NUM_TONES = 4;
constexpr int NUM_KEYS = 8;

constexpr PixelFormat DISPLAY_FORMAT = PixelFormat::RGB444;
FrameBuffer frameBuffer = createFrameBuffer(DISPLAY_FORMAT, true);

Mixer mixer = createMixer(NUM_TONES);
Tone tones[NUM_TONES];

uint64_t lastLoopTime = 0;
int keyboardMode = true;

int nextToneIndex = 0;
Button keys[] = {
    Button::DOWN, Button::LEFT, Button::UP, Button::RIGHT,
    Button::Y,    Button::X,    Button::A,  Button::B,
};
int keyToTone[] = {-1, -1, -1, -1, -1, -1, -1, -1};
int keyToNote[] = {-5, -3, -1, 0, 2, 4, 5, 7};

enum class ToneParamType {
  ENUM,
  INT32,
};

struct ParamEntry {
  ToneParamType type;
  const char *name;
  const void *unit;
  int min;
  int max;
  void *value;
};

const char *waveformNames[] = {
    "Square", "Sine", "Triangle", "Sawtooth", "Noise",
};

struct ToneParams {
  int waveform = (int)Waveform::SQUARE;
  int velocity = 64;
  int attackMs = 0;
  int decayMs = 1000;
  int sustainLevel = 192;
  int releaseMs = 500;
  int sweepDelta = 0;
  int sweepPeriodMs = 0;
  int keyShift = 64;
};

ToneParams params;

ParamEntry paramItems[] = {
    {ToneParamType::ENUM, "Waveform", waveformNames, 0,
     (int)Waveform::NUM_WAVEFORMS - 1, &params.waveform},
    {ToneParamType::INT32, "Velocity", "/ 127", 0, 127, &params.velocity},
    {ToneParamType::INT32, "Attack Time", "ms", 0, 1000, &params.attackMs},
    {ToneParamType::INT32, "Decay Time", "ms", 0, 1000, &params.decayMs},
    {ToneParamType::INT32, "Sustain Level", "/ 256", 0, 256,
     &params.sustainLevel},
    {ToneParamType::INT32, "Release Time", "ms", 0, 1000, &params.releaseMs},
    {ToneParamType::INT32, "Sweep Delta", "/ 65536", -32768, 65536,
     &params.sweepDelta},
    {ToneParamType::INT32, "Sweep Period", "ms", 0, 1000,
     &params.sweepPeriodMs},
    {ToneParamType::INT32, "Key Shift", "/ 127", 0, 127, &params.keyShift},
};

constexpr int NUM_TONE_PARAMS = sizeof(paramItems) / sizeof(paramItems[0]);
int selectedParamIndex = 0;
uint64_t paramAdjustStartMs = 0;
float paramAdjustAccum = 1.0f;

bool displayUpdateRequested = false;

void acceptKeyEvent(uint64_t nowMs, float dt);
void keyboardStateChanged(int ikey, bool pressed);
void requestDisplayUpdate();
void updateDisplay();

AppConfig xmcAppGetConfig(void) {
  AppConfig cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = DISPLAY_FORMAT;
  cfg.speakerEnabled = true;
  return cfg;
}

void xmcAppSetup(void) {
  frameBuffer->enableFlag(FrameBufferFlags::SHOW_DEBUG_INFO);

  // Display updates are event-driven, FPS is not needed
  frameBuffer->disableFlag(FrameBufferFlags::SHOW_FPS);

  // Create tone generators and set them as audio sources for the mixer.
  for (int i = 0; i < NUM_TONES; i++) {
    tones[i] = createTone();
    tones[i]->init();
    mixer->setSource(i, tones[i]->getOutputPort());
  }

  // Initialize key to tone mapping
  for (int i = 0; i < NUM_KEYS; i++) {
    keyToTone[i] = -1;
  }

  // Set the mixer output as the speaker source and unmute the speaker
  speaker::setSourcePort(mixer->getOutputPort());
  speaker::setMuted(false);

  requestDisplayUpdate();
}

void xmcAppLoop(void) {
  uint64_t nowMs = getTimeMs();
  float dt = (nowMs - lastLoopTime) / 1000.0f;
  lastLoopTime = nowMs;

  acceptKeyEvent(nowMs, dt);
  updateDisplay();
}

void acceptKeyEvent(uint64_t nowMs, float dt) {
  if (input::wasPressed(input::Button::FUNC)) {
    // Toggle mode
    keyboardMode = !keyboardMode;

    // Stop all tones
    for (int i = 0; i < NUM_KEYS; i++) {
      if (keyToTone[i] >= 0) {
        tones[keyToTone[i]]->noteOff();
        keyToTone[i] = -1;
      }
    }

    requestDisplayUpdate();
  }

  if (keyboardMode) {
    // In keyboard mode, each key directly triggers a tone
    for (int ikey = 0; ikey < NUM_KEYS; ikey++) {
      if (input::wasPressed(keys[ikey])) {
        keyboardStateChanged(ikey, true);
      }
      if (input::wasReleased(keys[ikey])) {
        keyboardStateChanged(ikey, false);
      }
    }
  } else {
    // In parameter mode, UP/DOWN keys select parameter, LEFT/RIGHT keys adjust
    // value

    // Select parameter with UP/DOWN keys
    if (input::wasPressed(input::Button::UP)) {
      selectedParamIndex--;
      if (selectedParamIndex < 0) {
        selectedParamIndex = NUM_TONE_PARAMS - 1;
      }
      requestDisplayUpdate();
    }
    if (input::wasPressed(input::Button::DOWN)) {
      selectedParamIndex++;
      if (selectedParamIndex >= NUM_TONE_PARAMS) {
        selectedParamIndex = 0;
      }
      requestDisplayUpdate();
    }

    ParamEntry &p = paramItems[selectedParamIndex];

    // Adjust value with LEFT/RIGHT keys
    int dir = 0;
    if (input::isPressed(input::Button::LEFT)) {
      dir -= 1;
    }
    if (input::isPressed(input::Button::RIGHT)) {
      dir += 1;
    }

    if (dir == 0) {
      // no direction, reset adjustment speed
      paramAdjustStartMs = nowMs;
      paramAdjustAccum = 1.0f;
    } else {
      // Accelerate adjustment speed based on dynamic range
      uint64_t elapsedMs = nowMs - paramAdjustStartMs;
      float speed = (p.max - p.min + 1) * elapsedMs * 0.00005f;
      paramAdjustAccum += speed * dt;
      int delta = (int)floorf(paramAdjustAccum);
      paramAdjustAccum -= delta;
      delta *= dir;

      // Apply the parameter change
      if (delta != 0) {
        switch (p.type) {
          case ToneParamType::ENUM: {
            int *val = (int *)p.value;
            *val = (int)((*val + delta + (p.max + 1)) % (p.max + 1));
          }; break;
          case ToneParamType::INT32: {
            int *val = (int *)p.value;
            *val = XMC_CLIP(p.min, p.max, *val + delta);
          }; break;
          default: break;
        }
        requestDisplayUpdate();
      }
    }

    // In parameter mode, 'A' button can also be used to trigger the selected
    // parameter for quick testing
    if (input::wasPressed(input::Button::A)) {
      keyboardStateChanged(3, true);
    } else if (input::wasReleased(input::Button::A)) {
      keyboardStateChanged(3, false);
    }
  }
}

void keyboardStateChanged(int ikey, bool pressed) {
  if (pressed) {
    // Key pressed
    if (keyToTone[ikey] >= 0) {
      // This key is already playing a tone, so stop it first
      tones[keyToTone[ikey]]->noteOff();
    }

    // Select next tone generator
    keyToTone[ikey] = nextToneIndex;
    nextToneIndex = (nextToneIndex + 1) % NUM_TONES;

    // Configure and start the tone
    Tone &t = tones[keyToTone[ikey]];
    t->setWaveform((Waveform)params.waveform);
    t->setVelocity(params.velocity);
    t->setEnvelope(params.attackMs, params.decayMs, params.sustainLevel,
                   params.releaseMs);
    t->setSweep(params.sweepDelta, params.sweepPeriodMs);
    t->noteOn(params.keyShift + keyToNote[ikey]);
  } else {
    // Key released
    if (keyToTone[ikey] >= 0) {
      tones[keyToTone[ikey]]->noteOff();
      keyToTone[ikey] = -1;
    }
  }
  requestDisplayUpdate();
}

void requestDisplayUpdate() { displayUpdateRequested = true; }

void updateDisplay() {
  if (!displayUpdateRequested) return;
  displayUpdateRequested = false;

  frameBuffer->beginRender();

  Graphics2D gfx = frameBuffer->createGraphics();
  gfx->clear(0);

  // Render menu items
  int menuItemHeight = 15;
  int menuHeight = NUM_TONE_PARAMS * menuItemHeight;
  int menuTop = display::HEIGHT - STATUS_BAR_HEIGHT - menuHeight;
  gfx->setFont(&ShapoSansP_s12c09a01w02, 1);
  gfx->setTextColor(0xFFF);
  for (int i = 0; i < NUM_TONE_PARAMS; i++) {
    ParamEntry &p = paramItems[i];

    int x = 0;
    int y = menuTop + i * menuItemHeight;

    if (!keyboardMode && i == selectedParamIndex) {
      // Fill background for selected item
      gfx->fillRect(0, y, display::WIDTH, menuItemHeight, 0x06C);
    }

    // Render parameter name
    x += 2;
    y += 2;
    gfx->setCursor(x, y);
    gfx->drawString(p.name);

    // Render parameter value
    x = display::WIDTH / 2;
    gfx->setCursor(x, y);
    switch (p.type) {
      case ToneParamType::ENUM: {
        // For enum value
        int val = *(int *)p.value;
        if (p.unit) {
          const char *const *enumNames = (const char *const *)p.unit;
          gfx->drawString(enumNames[val]);
        } else {
          char buf[16];
          snprintf(buf, sizeof(buf), "%d", val);
          gfx->drawString(buf);
        }
      } break;

      case ToneParamType::INT32: {
        // For integer value
        int val = *(int *)p.value;
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", val);
        gfx->drawString(buf);
        if (p.unit) {
          x = display::WIDTH * 3 / 4;
          gfx->setCursor(x, y);
          gfx->drawString((const char *)p.unit);
        }
      } break;
      default: break;
    }
  }

  // Keyboard layout
  int keyTop = STATUS_BAR_HEIGHT;
  int whiteKeyBottom = menuTop - 5;
  int whiteKeyHeight = whiteKeyBottom - keyTop;
  int blackKeyHeight = whiteKeyHeight * 3 / 5;

  // Paint white keys
  for (int i = 0; i < NUM_KEYS; i++) {
    int x = display::WIDTH * i / NUM_KEYS + 2;
    int w = display::WIDTH / NUM_KEYS - 4;
    gfx->fillRect(x, keyTop, w, whiteKeyHeight,
                  (keyToTone[i] < 0) ? 0xFFF : 0x06C);
  }

  // Paint black keys
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
    int w = display::WIDTH / 12;
    gfx->fillRect(x, keyTop, w, blackKeyHeight, 0x000);
  }

  frameBuffer->endRender();
}
