#ifndef XMC_GFX2D_EDGE_SCAN_HPP
#define XMC_GFX2D_EDGE_SCAN_HPP

#include <math.h>
#include "xmc/geo.hpp"
#include "xmc/gfx2d/gfx2d_common.hpp"

namespace xmc {

class EdgeScan {
 private:
  int iMin, iMax;
  int jMin, jMax;
  int i1, j1;
  int i2, j2;
  bool swapXY;
  int iStep;
  int i;
  int32_t jSlope;
  int32_t jFrac;
  int numSteps;

 public:
  XMC_INLINE EdgeScan(int x1, int y1, int x2, int y2, const Rect &clipRect) {
    numSteps = 0;

    i1 = x1;
    j1 = y1;
    i2 = x2;
    j2 = y2;
    iMin = clipRect.x;
    iMax = clipRect.right() - 1;
    jMin = clipRect.y;
    jMax = clipRect.bottom() - 1;

    swapXY = abs(j2 - j1) > abs(i2 - i1);
    if (swapXY) {
      std::swap(i1, j1);
      std::swap(i2, j2);
      std::swap(iMin, jMin);
      std::swap(iMax, jMax);
    }

    int iRange = i2 - i1;
    int jRange = j2 - j1;

    iStep = iRange > 0 ? 1 : -1;
    jSlope = jRange * 0x10000 / abs(iRange);
    jFrac = j1 * 0x10000;

    if (i1 < iMin) {
      if (iStep <= 0) return;
      jFrac += jSlope * abs(iMin - i1);
      i1 = iMin;
      j1 = jFrac >> 16;
    } else if (i1 > iMax) {
      if (iStep >= 0) return;
      jFrac += jSlope * abs(i1 - iMax);
      i1 = iMax;
      j1 = jFrac >> 16;
    }

    if (i2 < iMin) {
      if (iStep >= 0) return;
      i2 = iMin;
    } else if (i2 > iMax) {
      if (iStep <= 0) return;
      i2 = iMax;
    }

#if 0
  // todo: debug
  if (j1 < jMin) {
    if (jSlope <= 0) return;
    int iDelta = (jMin - j1) * 0x10000 / jSlope;
    jFrac += jSlope * iDelta;
    i1 += iDelta;
    j1 = jFrac >> 16;
  } else if (j1 > jMax) {
    if (jSlope >= 0) return;
    int iDelta = (jMax - j1) * 0x10000 / jSlope;
    jFrac += jSlope * iDelta;
    i1 += iDelta;
    j1 = jFrac >> 16;
  }

  if (j2 < jMin) {
    if (jSlope >= 0) return;
    int iDelta = (jMin - j2) * 0x10000 / jSlope;
    jFrac += jSlope * iDelta;
    i2 += iDelta;
    j2 = jFrac >> 16;
  } else if (j2 > jMax) {
    if (jSlope <= 0) return;
    int iDelta = (jMax - j2) * 0x10000 / jSlope;
    jFrac += jSlope * iDelta;
    i2 += iDelta;
    j2 = jFrac >> 16;
  }
#endif

    i = i1;
    numSteps = (i2 - i1) / iStep;
  }

  XMC_INLINE bool yieldPixel(int *px, int *py) {
    int j;
    do {
      if (numSteps <= 0) return false;
      j = (jFrac + 0x8000) >> 16;
      jFrac += jSlope;
      i += iStep;
      numSteps--;
    } while (j < jMin || j > jMax);

    if (swapXY) {
      *px = j;
      *py = i;
    } else {
      *px = i;
      *py = j;
    }
    return true;
  }
};

}  // namespace xmc

#endif
