/**
 * @file tone.hpp
 */

#ifndef XMC_AUDIO_TONE_HPP
#define XMC_AUDIO_TONE_HPP

#include "xmc/audio_common.hpp"

#include <math.h>
#include <stdint.h>
#include <memory>

namespace xmc::audio {

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
  /** noise */
  NOISE,
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
class ToneClass {
 private:
  uint32_t rateHz = 44100;
  uint32_t tickPhaseStep = 0;

  Waveform waveform = Waveform::SQUARE;
  uint8_t velocity = 0;
  uint32_t tonePhaseStep = 0;
  uint32_t attackMs = 0;
  uint32_t decayMs = 0;
  uint16_t sustainLevel = 0;
  uint32_t releaseMs = 0;

  uint16_t tonePhaseCounter = 0;
  uint16_t noiseLfsr = 0xFFFF;
  uint32_t lengthCounter = 0;
  EnvelopeState envelopeState = EnvelopeState::IDLE;
  uint32_t envelopeCounter = 0;
  uint16_t envelopeAmpInit = 0;
  uint16_t envelopeAmpCurr = 0;
  uint32_t tickPhaseCounter = 0;
  uint32_t sweepCoeff = 0x10000;
  uint32_t sweepPeriodMs = 0;
  uint32_t sweepCounter = 0;

  SourcePort outputPort;

 public:
  ToneClass();

  /**
   * Initializes the tone generator with the given sample rate.
   * @param rateHz The sample rate in Hz.
   */
  void init(uint32_t rateHz = 0);

  /**
   * Sets the waveform of the tone.
   * @param wf The waveform to set. This determines the shape of the sound wave
   * and thus the timbre of the sound.
   */
  inline void setWaveform(Waveform wf) { waveform = wf; }

  /**
   * Gets the current waveform of the tone.
   * @return The current waveform of the tone.
   */
  inline Waveform getWaveform() const { return waveform; }

  /**
   * Sets the velocity (volume) of the tone.
   * @param velo The velocity to set. The velocity must be between 0 and 127.
   */
  inline void setVelocity(uint8_t velo) { velocity = XMC_CLIP(0, 127, velo); }

  /**
   * Gets the current velocity (volume) of the tone.
   * @return The current velocity of the tone.
   */
  inline uint8_t getVelocity() const { return velocity; }

  /**
   * Sets the envelope of the tone. The envelope controls the amplitude of the
   * tone over time, allowing for more natural-sounding tones with attack,
   * decay, sustain, and release phases.
   * @param attackMs The duration of the attack phase in milliseconds.
   * @param decayMs The duration of the decay phase in milliseconds.
   * @param sustainLevel The sustain level as a value between 0 and 256.
   * @param releaseMs The duration of the release phase in milliseconds.
   */
  inline void setEnvelope(uint16_t attackMs, uint16_t decayMs,
                          uint16_t sustainLevel, uint16_t releaseMs) {
    this->attackMs = attackMs;
    this->decayMs = decayMs;
    this->sustainLevel = sustainLevel;
    this->releaseMs = releaseMs;
  }

  /**
   * Sets the frequency sweep of the tone. The frequency sweep allows for
   * effects such as siren-like sounds.
   * @param delta The amount of frequency change for the sweep. This is a signed
   * fixed-point number with 16-bit fractional part. The valid range is from
   * -32768 (0.5) to 65536 (2.0). When delta is 0, the sweep function is
   * disabled. When delta is non-zero, the tone frequency is multiplied by
   * (1.0 + delta / 65536.0) at each sweep step. For example, setting delta to
   * 3897 (2 ^ (1.0 / 12.0) - 1) will increase the frequency by a semitone at
   * each step, resulting in approximately one octave increase after 12 steps.
   * @param period_ms The period of the sweep in milliseconds. This determines
   * how fast the frequency changes over time.
   */
  inline void setSweep(int32_t delta, uint32_t period_ms) {
    if (delta < -32768) {
      delta = -32768;
    } else if (delta > 65536) {
      delta = 65536;
    }
    sweepCoeff = 0x10000 + delta;
    sweepPeriodMs = period_ms;
    sweepCounter = 0;
  }

  /**
   * Starts playing a note with the given MIDI note number and length. The MIDI
   * note number determines the frequency of the tone, with 69 representing A4
   * (440 Hz). The length determines how long the note will play before it is
   * automatically turned off. If the length is set to TONE_LENGTH_INFINITE, the
   * note will play indefinitely until it is manually turned off with noteOff()
   * or muted with mute().
   * @param note The MIDI note number to play. The MIDI note number must be
   * between 0 and 127, where 69 represents A4 (440 Hz).
   * @param lenMs The length of the note in milliseconds. If set to
   * TONE_LENGTH_INFINITE, the note will play indefinitely until it is manually
   * turned off with noteOff() or muted with mute().
   */
  void noteOn(uint8_t note, uint32_t lenMs = TONE_LENGTH_INFINITE);

  /**
   * Starts playing a note with the given frequency and length. The frequency is
   * specified in Hz. The length determines how long the note will play before
   * it is automatically turned off. If the length is set to
   * TONE_LENGTH_INFINITE, the note will play indefinitely until it is manually
   * turned off with noteOff() or muted with mute().
   * @param freq The frequency of the note to play in Hz.
   * @param lenMs The length of the note in milliseconds. If set to
   * TONE_LENGTH_INFINITE, the note will play indefinitely until it is manually
   * turned off with noteOff() or muted with mute().
   */
  void noteOnWithFreq(uint32_t freq, uint32_t lenMs = TONE_LENGTH_INFINITE);

  /**
   * Stops playing the current note. This will trigger the release phase of the
   * envelope, allowing the note to fade out naturally according to the release
   * time set in the envelope. If the note is currently in the attack, decay, or
   * sustain phase, it will transition to the release phase. If the note is
   * already in the release phase or idle, this function will have no effect.
   */
  void noteOff();

  /**
   * Mutes the tone immediately. This will stop all sound output from the tone
   * immediately, without going through the release phase of the envelope.
   */
  void mute();

  /**
   * Renders the audio samples for the current state of the tone into the
   * provided buffer. The number of samples to render is determined by the
   * numSamples parameter, and the samples will be written to the buffer as
   * 16-bit signed integers.
   * @param buffer The buffer to write the audio samples to.
   * @param numSamples The number of audio samples to render into the buffer.
   */
  void render(int16_t *buffer, uint32_t numSamples);

  /**
   * Returns a pointer to the output port of the tone. The output port can be
   * connected to the speaker or to a mixer for further processing.
   * @return A pointer to the output port of the tone.
   */
  SourcePort *getOutputPort() { return &outputPort; }

 private:
  void tick();
};

using Tone = std::shared_ptr<ToneClass>;

static inline Tone createTone() { return std::make_shared<ToneClass>(); }

}  // namespace xmc::audio

#endif
