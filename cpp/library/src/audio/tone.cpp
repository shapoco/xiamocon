#include "xmc/audio/tone.hpp"
#include "xmc/speaker.hpp"

namespace xmc::audio {

static void xmc_tone_request_data(void *buffer, uint32_t size_bytes,
                                  void *context);

ToneClass::ToneClass() {
  outputPort.requestData = xmc_tone_request_data;
  outputPort.context = this;
}

void ToneClass::init(uint32_t rate_hz) {
  if (rate_hz == 0) {
    rate_hz = speaker::getStreamFormat().rateHz;
  }
  rateHz = rate_hz;
  tickPhaseStep = 0x10000UL * 1000 / rateHz;

  waveform = Waveform::SQUARE;
  velocity = 64;
  attackMs = 0;
  decayMs = 0;
  sustainLevel = 256;
  releaseMs = 0;

  tonePhaseStep = 0;
  noiseLfsr = 0xFFFF;
  tonePhaseCounter = 0;
  lengthCounter = 0;
  envelopeState = EnvelopeState::IDLE;
  envelopeCounter = 0;
  envelopeAmpInit = 0;
  envelopeAmpCurr = 0;
  tickPhaseCounter = 0;
}

void ToneClass::noteOn(uint8_t note, uint32_t lenMs) {
  // todo: optimize
  uint32_t freq = 440 * pow(2, (note - 69) / 12.0);
  noteOnWithFreq(freq, lenMs);
}

void ToneClass::noteOnWithFreq(uint32_t freq, uint32_t lenMs) {
  if (freq == 0) {
    mute();
    return;
  }

  tonePhaseStep = (uint64_t)freq * 0x10000 / rateHz;
  lengthCounter = lenMs;
  tickPhaseCounter = 0;
  tonePhaseCounter = 0;

  if (attackMs > 0) {
    envelopeState = EnvelopeState::ATTACK;
    envelopeCounter = 0;
    envelopeAmpInit = 0;
    envelopeAmpCurr = 0;
  } else {
    if (lenMs == 0) {
      envelopeState = EnvelopeState::RELEASE;
    } else if (decayMs > 0) {
      envelopeState = EnvelopeState::DECAY;
    } else {
      envelopeState = EnvelopeState::SUSTAIN;
    }
    envelopeCounter = 0;
    envelopeAmpInit = (uint32_t)velocity * (0x10000 / 128);
    envelopeAmpCurr = envelopeAmpInit;
  }
}

void ToneClass::noteOff() {
  if (envelopeState != EnvelopeState::IDLE &&
      envelopeState != EnvelopeState::RELEASE) {
    if (releaseMs > 0) {
      envelopeState = EnvelopeState::RELEASE;
      envelopeCounter = 0;
      envelopeAmpInit = envelopeAmpCurr;
    } else {
      envelopeState = EnvelopeState::IDLE;
    }
  }
}

void ToneClass::mute() {
  envelopeState = EnvelopeState::IDLE;
  lengthCounter = 0;
}

void ToneClass::render(int16_t *buffer, uint32_t numSamples) {
  if (envelopeState == EnvelopeState::IDLE) {
    return;
  }

  for (uint32_t i = 0; i < numSamples; i++) {
    tickPhaseCounter += tickPhaseStep;
    if (tickPhaseCounter >= 0x10000) {
      tickPhaseCounter -= 0x10000;
      tick();
    }

    const int OVERSAMPLING = 1;
    int32_t raw = 0;
    for (int i = 0; i < OVERSAMPLING; i++) {
      uint32_t p = tonePhaseCounter + i * tonePhaseStep / OVERSAMPLING;
      switch (waveform) {
        default:
        case Waveform::SQUARE: raw += (p < 0x8000) ? 0x8000 : -0x8000; break;
        case Waveform::SINE:
          raw += (sinf((p / 65536.0) * 2 * M_PI) * 0x8000);
          break;
        case Waveform::TRIANGLE:
          if (p < 0x8000) {
            raw += p * 2;
          } else {
            raw += (0x10000 - p) * 2;
          }
          raw -= 0x8000;
          break;
        case Waveform::SAWTOOTH: raw += p - 0x8000; break;
        case Waveform::NOISE: raw += (int32_t)noiseLfsr - 0x8000; break;
      }
    }
    raw /= OVERSAMPLING;

    uint16_t last_phase = tonePhaseCounter;
    tonePhaseCounter += tonePhaseStep;
    if (waveform == Waveform::NOISE &&
        (last_phase & 0x400) != (tonePhaseCounter & 0x400)) {
      noiseLfsr = (noiseLfsr >> 1) ^ (-(noiseLfsr & 1) & 0xB400);
    }

    uint32_t amp = envelopeAmpCurr;
    amp = (amp * amp) / 0x10000;
    buffer[i] += (raw * amp / 0x10000);
  }
}

void ToneClass::tick() {
  if (lengthCounter != TONE_LENGTH_INFINITE && lengthCounter > 0) {
    lengthCounter--;
    if (lengthCounter == 0) {
      noteOff();
    }
  }

  switch (envelopeState) {
    case EnvelopeState::IDLE: break;

    case EnvelopeState::ATTACK:
      envelopeCounter++;
      if (envelopeCounter < attackMs) {
        envelopeAmpCurr =
            (uint32_t)velocity * (0x10000 / 128) * envelopeCounter / attackMs;
      } else {
        envelopeState =
            decayMs > 0 ? EnvelopeState::DECAY : EnvelopeState::SUSTAIN;
        envelopeCounter = 0;
        envelopeAmpCurr = (uint32_t)velocity * (0x10000 / 128);
      }
      break;

    case EnvelopeState::DECAY: {
      envelopeCounter++;
      uint32_t start = (uint32_t)velocity * (0x10000 / 128);
      uint32_t goal = start * sustainLevel / 256;
      if (envelopeCounter < decayMs) {
        envelopeAmpCurr = start - ((start - goal) * envelopeCounter / decayMs);
      } else {
        envelopeState = EnvelopeState::SUSTAIN;
        envelopeCounter = 0;
        envelopeAmpCurr = goal;
      }
    } break;

    case EnvelopeState::SUSTAIN: break;

    case EnvelopeState::RELEASE:
      envelopeCounter++;
      if (envelopeCounter < releaseMs) {
        envelopeAmpCurr =
            envelopeAmpInit - (envelopeAmpInit * envelopeCounter / releaseMs);
      } else {
        envelopeState = EnvelopeState::IDLE;
        envelopeCounter = 0;
        envelopeAmpCurr = 0;
      }
      break;
  }

  if (sweepPeriodMs > 0) {
    sweepCounter++;
    if (sweepCounter >= sweepPeriodMs) {
      sweepCounter = 0;
      tonePhaseStep = (uint64_t)tonePhaseStep * sweepCoeff / 0x10000;
    }
  }
}

static void xmc_tone_request_data(void *buffer, uint32_t numSamples,
                                  void *context) {
  ((ToneClass *)context)->render((int16_t *)buffer, numSamples);
}

}  // namespace xmc::audio
