#include "xmc/fps_keeper.hpp"

namespace xmc {

void FpsKeeper::waitVsync() {
  uint64_t now = getTimeUs();
  int64_t sleepTimeUs = nextVsyncTimeUs - now;
  if (sleepTimeUs > 0) {
    sleepUs(sleepTimeUs);
  }
  nextVsyncTimeUs += 1000000 / targetFps;
  bool delayed = nextVsyncTimeUs < now - maxJitterUs;
  if (delayed) {
    nextVsyncTimeUs = now;
  }
  frameSkipping = delayed && !frameSkipping;
}

}  // namespace xmc
