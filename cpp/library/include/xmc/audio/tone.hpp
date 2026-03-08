#ifndef XMC_AUDIO_TONE_HPP
#define XMC_AUDIO_TONE_HPP

#include "xmc/audio_common.h"

#include <math.h>
#include <stdint.h>

namespace xmc {

enum class Waveform {
  SQUARE,
  SINE,
  TRIANGLE,
  SAWTOOTH,
};

enum class EnvelopeState {
  IDLE,
  ATTACK,
  DECAY,
  SUSTAIN,
  RELEASE,
};

static constexpr uint32_t TONE_LENGTH_INFINITE = 0xFFFFFFFF;

class Tone {
 private:
  uint32_t sample_rate_hz = 44100;
  uint32_t tick_phase_step = 0;

  Waveform waveform = Waveform::SQUARE;
  uint8_t velocity = 0;
  uint32_t tone_phase_step = 0;
  uint32_t attack_ms = 0;
  uint32_t decay_ms = 0;
  uint16_t sustain_level = 0;
  uint32_t release_ms = 0;
  uint16_t tone_phase_counter = 0;
  uint32_t length_counter = 0;
  EnvelopeState envelope_state = EnvelopeState::IDLE;
  uint32_t envelope_counter = 0;
  uint16_t envelope_amp_init = 0;
  uint16_t envelope_amp_curr = 0;
  uint32_t tick_phase_counter = 0;
  uint32_t sweep_coeff = 0x10000;
  uint32_t sweep_period_ms = 0;
  uint32_t sweep_counter = 0;

 public:
  void init(uint32_t sample_rate_hz);

  inline void set_waveform(Waveform wf) { waveform = wf; }

  inline void set_velocity(uint8_t velo) {
    if (velo > 127) {
      velo = 127;
    }
    velocity = velo;
  }

  inline void set_envelope(uint16_t attack_ms, uint16_t decay_ms,
                           uint16_t sustain_level, uint16_t release_ms) {
    this->attack_ms = attack_ms;
    this->decay_ms = decay_ms;
    this->sustain_level = sustain_level;
    this->release_ms = release_ms;
  }

  inline void set_sweep(int32_t delta, uint32_t period_ms) {
    if (delta < -0x8000) {
      delta = -0x8000;
    } else if (delta > 0x8000) {
      delta = 0x8000;
    }
    sweep_coeff = 0x10000 + delta;
    sweep_period_ms = period_ms;
    sweep_counter = 0;
  }

  void note_on(uint8_t midi_note, uint32_t len_ms = TONE_LENGTH_INFINITE);

  void note_on_with_freq(uint32_t freq, uint32_t len_ms = TONE_LENGTH_INFINITE);

  void note_off();

  void mute();

  void render(uint16_t *buffer, uint32_t num_samples);

  xmc_stream_source_t get_output();

 private:
  void tick();
};

}  // namespace xmc

#endif
