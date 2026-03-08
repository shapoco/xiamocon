/**
 * @file tone.hpp
 */

#ifndef XMC_AUDIO_TONE_HPP
#define XMC_AUDIO_TONE_HPP

#include "xmc/audio_common.h"

#include <math.h>
#include <stdint.h>

namespace xmc {

/**
 * The waveform of the tone. This determines the shape of the sound wave and
 * thus the timbre of the sound.
 */
enum class Waveform {
  /** square wave */
  SQUARE,
  /** sine wave */
  SINE,
  /** triangle wave */
  TRIANGLE,
  /** sawtooth wave */
  SAWTOOTH,
  /** number of waveforms */
  NUM_WAVEFORMS,
};

/**
 * The state of the envelope generator. The envelope generator controls the
 * amplitude of the tone over time, allowing for more natural-sounding tones
 * with attack, decay, sustain, and release phases.
 */
enum class EnvelopeState {
  /** The envelope is idle */
  IDLE,
  /** The envelope is in the attack phase */
  ATTACK,
  /** The envelope is in the decay phase */
  DECAY,
  /** The envelope is in the sustain phase */
  SUSTAIN,
  /** The envelope is in the release phase */
  RELEASE,
};

static constexpr uint32_t TONE_LENGTH_INFINITE = 0xFFFFFFFF;

/**
 * A simple audio tone generator that can generate a tone with a given
 * frequency, length, and envelope. The tone can be used as an audio source for
 * the speaker or for mixing with other tones using the mixer.
 */
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

  xmc_audio_source_port_t output_port;

 public:
  Tone();

  /**
   * Initializes the tone generator with the given sample rate.
   * @param sample_rate_hz The sample rate in Hz.
   */
  void init(uint32_t sample_rate_hz);

  /**
   * Sets the waveform of the tone.
   * @param wf The waveform to set. This determines the shape of the sound wave
   * and thus the timbre of the sound.
   */
  inline void set_waveform(Waveform wf) { waveform = wf; }

  /**
   * Sets the velocity (volume) of the tone.
   * @param velo The velocity to set. The velocity must be between 0 and 127.
   */
  inline void set_velocity(uint8_t velo) {
    if (velo > 127) {
      velo = 127;
    }
    velocity = velo;
  }

  /**
   * Sets the envelope of the tone. The envelope controls the amplitude of the
   * tone over time, allowing for more natural-sounding tones with attack,
   * decay, sustain, and release phases.
   * @param attack_ms The duration of the attack phase in milliseconds.
   * @param decay_ms The duration of the decay phase in milliseconds.
   * @param sustain_level The sustain level as a value between 0 and 256.
   * @param release_ms The duration of the release phase in milliseconds.
   */
  inline void set_envelope(uint16_t attack_ms, uint16_t decay_ms,
                           uint16_t sustain_level, uint16_t release_ms) {
    this->attack_ms = attack_ms;
    this->decay_ms = decay_ms;
    this->sustain_level = sustain_level;
    this->release_ms = release_ms;
  }

  /**
   * Sets the frequency sweep of the tone. The frequency sweep allows for
   * effects such as siren-like sounds.
   * @param delta The amount of frequency change for the sweep. This is a
   * fixed-point value where 0x10000 represents no change, values less than
   * 0x10000 represent a decrease in frequency, and values greater than 0x10000
   * represent an increase in frequency.
   * @param period_ms The period of the sweep in milliseconds. This determines
   * how fast the frequency changes over time.
   */
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

  /**
   * Starts playing a note with the given MIDI note number and length. The MIDI
   * note number determines the frequency of the tone, with 69 representing A4
   * (440 Hz). The length determines how long the note will play before it is
   * automatically turned off. If the length is set to TONE_LENGTH_INFINITE, the
   * note will play indefinitely until it is manually turned off with note_off()
   * or muted with mute().
   * @param midi_note The MIDI note number to play. The MIDI note number must be
   * between 0 and 127, where 69 represents A4 (440 Hz).
   * @param len_ms The length of the note in milliseconds. If set to
   * TONE_LENGTH_INFINITE, the note will play indefinitely until it is manually
   * turned off with note_off() or muted with mute().
   */
  void note_on(uint8_t midi_note, uint32_t len_ms = TONE_LENGTH_INFINITE);

  /**
   * Starts playing a note with the given frequency and length. The frequency is
   * specified in Hz. The length determines how long the note will play before
   * it is automatically turned off. If the length is set to
   * TONE_LENGTH_INFINITE, the note will play indefinitely until it is manually
   * turned off with note_off() or muted with mute().
   * @param freq The frequency of the note to play in Hz.
   * @param len_ms The length of the note in milliseconds. If set to
   * TONE_LENGTH_INFINITE, the note will play indefinitely until it is manually
   * turned off with note_off() or muted with mute().
   */
  void note_on_with_freq(uint32_t freq, uint32_t len_ms = TONE_LENGTH_INFINITE);

  /**
   * Stops playing the current note. This will trigger the release phase of the
   * envelope, allowing the note to fade out naturally according to the release
   * time set in the envelope. If the note is currently in the attack, decay, or
   * sustain phase, it will transition to the release phase. If the note is
   * already in the release phase or idle, this function will have no effect.
   */
  void note_off();

  /**
   * Mutes the tone immediately. This will stop all sound output from the tone
   * immediately, without going through the release phase of the envelope.
   */
  void mute();

  /**
   * Renders the audio samples for the current state of the tone into the
   * provided buffer. The number of samples to render is determined by the
   * num_samples parameter, and the samples will be written to the buffer as
   * 16-bit signed integers.
   * @param buffer The buffer to write the audio samples to.
   * @param num_samples The number of audio samples to render into the buffer.
   */
  void render(int16_t *buffer, uint32_t num_samples);

  /**
   * Returns a pointer to the output port of the tone. The output port can be
   * connected to the speaker or to a mixer for further processing.
   * @return A pointer to the output port of the tone.
   */
  xmc_audio_source_port_t *get_output_port() { return &output_port; }

 private:
  void tick();
};

}  // namespace xmc

#endif
