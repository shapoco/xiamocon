/**
 * @file fps_keeper.hpp
 * @brief Frame rate keeper for synchronizing rendering to a target FPS
 */

#ifndef XMC_GFX_FPS_KEEPER_HPP
#define XMC_GFX_FPS_KEEPER_HPP

#include "xmc/timer.hpp"

namespace xmc {

class FpsKeeper {
 private:
  float targetFps;
  uint64_t nextVsyncTimeUs;
  uint32_t maxJitterUs;
  bool frameSkipping = false;

 public:
  FpsKeeper(float fps, uint32_t maxJitter = 5000)
      : targetFps(fps), nextVsyncTimeUs(getTimeUs()), maxJitterUs(maxJitter) {}

  /**
   * @brief Set the target frames per second (FPS) for the frame rate keeper
   * @param fps The desired target FPS (e.g., 60.0 for 60 FPS)
   */
  inline void setTargetFps(float fps) { targetFps = fps; }

  /**
   * @brief Wait until the next pseudo vertical sync (VSync) time based on the
   * target FPS This function calculates the time until the next VSync should
   * occur based on the target FPS and the last VSync time.
   */
  void waitVsync();

  /**
   * @brief Check if the last frame was delayed beyond the maximum allowed
   * jitter
   * @return true if the last frame was delayed and frame skipping is active,
   * false if the last frame was on time or if frame skipping is not active
   */
  inline bool isFrameSkipping() const { return frameSkipping; }
};

}  // namespace xmc

#endif  // XMC_GFX_FPS_KEEPER_HPP