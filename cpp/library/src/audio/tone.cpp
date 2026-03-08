#include "xmc/audio/tone.hpp"

namespace xmc {

static void xmc_tone_request_data(void *buffer, uint32_t size_bytes,
                                  void *context);

void Tone::init(uint32_t rate_hz) {
  sample_rate_hz = rate_hz;
  tick_phase_step = 0x10000UL * 1000 / sample_rate_hz;

  waveform = Waveform::SQUARE;
  velocity = 64;
  attack_ms = 0;
  decay_ms = 0;
  sustain_level = 256;
  release_ms = 0;

  tone_phase_step = 0;
  tone_phase_counter = 0;
  length_counter = 0;
  envelope_state = EnvelopeState::IDLE;
  envelope_counter = 0;
  envelope_amp_init = 0;
  envelope_amp_curr = 0;
  tick_phase_counter = 0;
}

void Tone::note_on(uint8_t midi_note, uint32_t len_ms) {
  // todo: optimize
  uint32_t freq = 440 * pow(2, (midi_note - 69) / 12.0);
  note_on_with_freq(freq, len_ms);
}

void Tone::note_on_with_freq(uint32_t freq, uint32_t len_ms) {
  if (freq == 0) {
    mute();
    return;
  }

  tone_phase_step = (uint64_t)freq * 0x10000 / sample_rate_hz;
  length_counter = len_ms;
  if (attack_ms > 0) {
    envelope_state = EnvelopeState::ATTACK;
    envelope_counter = 0;
    envelope_amp_init = 0;
    envelope_amp_curr = 0;
  } else {
    if (len_ms == 0) {
      envelope_state = EnvelopeState::RELEASE;
    } else if (decay_ms > 0) {
      envelope_state = EnvelopeState::DECAY;
    } else {
      envelope_state = EnvelopeState::SUSTAIN;
    }
    envelope_counter = 0;
    envelope_amp_init = (uint32_t)velocity * (0x10000 / 128);
    envelope_amp_curr = envelope_amp_init;
  }
}

void Tone::note_off() {
  if (envelope_state != EnvelopeState::IDLE &&
      envelope_state != EnvelopeState::RELEASE) {
    if (release_ms > 0) {
      envelope_state = EnvelopeState::RELEASE;
      envelope_counter = 0;
      envelope_amp_init = envelope_amp_curr;
    } else {
      envelope_state = EnvelopeState::IDLE;
    }
  }
}

void Tone::mute() {
  envelope_state = EnvelopeState::IDLE;
  length_counter = 0;
}

void Tone::render(uint16_t *buffer, uint32_t num_samples) {
  if (envelope_state == EnvelopeState::IDLE) {
    for (uint32_t i = 0; i < num_samples; i++) {
      buffer[i] = 0x8000;
    }
    return;
  }

  for (uint32_t i = 0; i < num_samples; i++) {
    tick_phase_counter += tick_phase_step;
    if (tick_phase_counter >= 0x10000) {
      tick_phase_counter -= 0x10000;
      tick();
    }

    const int OVERSAMPLING = 1;
    int32_t raw = 0;
    for (int i = 0; i < OVERSAMPLING; i++) {
      uint32_t p = tone_phase_counter + i * tone_phase_step / OVERSAMPLING;
      switch (waveform) {
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
          //if (raw < 0x800) {
          //  raw = 0x800;
          //}
          //else if (raw > 0xF800) {
          //  raw = 0xF800;
          //}
          raw -= 0x8000;
          break;
        case Waveform::SAWTOOTH: raw += p - 0x8000; break;
      }
    }
    raw /= OVERSAMPLING;

    tone_phase_counter += tone_phase_step;

    uint32_t amp = envelope_amp_curr;
    amp = (amp * amp) / 0x10000;
    buffer[i] = 0x8000 + (raw * amp / 0x10000);
  }
}

xmc_stream_source_t Tone::get_output() {
  xmc_stream_source_t source;
  source.request_data = xmc_tone_request_data;
  source.context = this;
  return source;
}

void Tone::tick() {
  if (length_counter != TONE_LENGTH_INFINITE && length_counter > 0) {
    length_counter--;
    if (length_counter == 0) {
      note_off();
    }
  }

  switch (envelope_state) {
    case EnvelopeState::IDLE: break;

    case EnvelopeState::ATTACK:
      envelope_counter++;
      if (envelope_counter < attack_ms) {
        envelope_amp_curr =
            (uint32_t)velocity * (0x10000 / 128) * envelope_counter / attack_ms;
      } else {
        envelope_state =
            decay_ms > 0 ? EnvelopeState::DECAY : EnvelopeState::SUSTAIN;
        envelope_counter = 0;
        envelope_amp_curr = (uint32_t)velocity * (0x10000 / 128);
      }
      break;

    case EnvelopeState::DECAY: {
      envelope_counter++;
      uint32_t start = (uint32_t)velocity * (0x10000 / 128);
      uint32_t goal = start * sustain_level / 256;
      if (envelope_counter < decay_ms) {
        envelope_amp_curr =
            start - ((start - goal) * envelope_counter / decay_ms);
      } else {
        envelope_state = EnvelopeState::SUSTAIN;
        envelope_counter = 0;
        envelope_amp_curr = goal;
      }
    } break;

    case EnvelopeState::SUSTAIN: break;

    case EnvelopeState::RELEASE:
      envelope_counter++;
      if (envelope_counter < release_ms) {
        envelope_amp_curr = envelope_amp_init -
                            (envelope_amp_init * envelope_counter / release_ms);
      } else {
        envelope_state = EnvelopeState::IDLE;
        envelope_counter = 0;
        envelope_amp_curr = 0;
      }
      break;
  }

  if (sweep_period_ms > 0) {
    sweep_counter++;
    if (sweep_counter >= sweep_period_ms) {
      sweep_counter = 0;
      tone_phase_step = (uint64_t)tone_phase_step * sweep_coeff / 0x10000;
    }
  }
}

static void xmc_tone_request_data(void *buffer, uint32_t num_samples,
                                  void *context) {
  ((Tone *)context)->render((uint16_t *)buffer, num_samples);
}

}  // namespace xmc
