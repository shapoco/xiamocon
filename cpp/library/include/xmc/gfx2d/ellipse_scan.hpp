#ifndef XMC_GFX2D_ELLIPSE_SCAN_HPP
#define XMC_GFX2D_ELLIPSE_SCAN_HPP

#include <math.h>
#include "xmc/geo.hpp"
#include "xmc/gfx2d/gfx2d_common.hpp"

namespace xmc {

class EllipseScan {
 private:
  int cx2, cy2;
  int rx2, ry2;
  int xMin, xMax;
  int yMin, yMax;
  int y;

 public:
  XMC_INLINE EllipseScan(Rect dstRect, const Rect &clipRect) {
    dstRect.normalizeInPlace();
    cx2 = dstRect.x * 2 + dstRect.width - 1;
    cy2 = dstRect.y * 2 + dstRect.height - 1;
    rx2 = dstRect.width - 1;
    ry2 = dstRect.height - 1;
    xMin = clipRect.x;
    xMax = clipRect.right() - 1;
    yMin = XMC_CLIP(clipRect.y, clipRect.bottom() - 1, dstRect.y);
    yMax = XMC_CLIP(clipRect.y, clipRect.bottom() - 1, dstRect.bottom() - 1);
    y = yMin - 1;
  }

  XMC_INLINE bool nextLine(int *py, int *px, int *pw) {
    int dx2, dy2;
    do {
      if (++y > yMax) return false;

      dy2 = y * 2 - cy2;
      if (ry2 == 0) {
        dx2 = rx2;
      } else {
        float t = 1.0f - (dy2 * dy2) / (float)(ry2 * ry2);
        if (t < 0.0f) {
          continue;
        }
        dx2 = (int)roundf(rx2 * sqrtf(t));
      }

      int x1 = (cx2 - dx2) / 2;
      int x2 = (cx2 + dx2) / 2;

      int clipX1 = x1 > xMin ? x1 : xMin;
      int clipX2 = x2 < xMax ? x2 : xMax;
      int w = clipX2 - clipX1 + 1;
      if (w <= 0) continue;

      *px = clipX1;
      *py = y;
      *pw = w;
    } while (false);
    return true;
  }
};

}  // namespace xmc

#endif
