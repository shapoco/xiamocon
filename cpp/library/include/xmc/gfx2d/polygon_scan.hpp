#ifndef XMC_GFX2D_POLYGON_SCAN_HPP
#define XMC_GFX2D_POLYGON_SCAN_HPP

#include <math.h>
#include "xmc/geo.hpp"
#include "xmc/gfx2d/gfx2d_common.hpp"

namespace xmc {

class PolygonScan {
  static constexpr int MAX_CROSSES = 8;

 private:
  const vec2i *vertices;
  int numVertices;
  int xMin, xMax;
  int yMin, yMax;
  int xCrosses[MAX_CROSSES];
  int numCrosses;
  int nextCrossIndex;
  int y;

 public:
  XMC_INLINE PolygonScan(const vec2i *vertices, int numVertices,
                         const Rect &clipRect)
      : vertices(vertices), numVertices(numVertices) {
    xMin = xMax = vertices[0].x;
    yMin = yMax = vertices[0].y;
    for (int i = 1; i < numVertices; i++) {
      if (vertices[i].x < xMin) xMin = vertices[i].x;
      if (vertices[i].x > xMax) xMax = vertices[i].x;
      if (vertices[i].y < yMin) yMin = vertices[i].y;
      if (vertices[i].y > yMax) yMax = vertices[i].y;
    }
    xMin = XMC_CLIP(clipRect.x, clipRect.right() - 1, xMin);
    xMax = XMC_CLIP(clipRect.x, clipRect.right() - 1, xMax);
    yMin = XMC_CLIP(clipRect.y, clipRect.bottom() - 1, yMin);
    yMax = XMC_CLIP(clipRect.y, clipRect.bottom() - 1, yMax);
    y = yMin - 1;
    numCrosses = 0;
    nextCrossIndex = 0;
  }

  XMC_INLINE Rect getBoundingBox() const {
    return Rect{xMin, yMin, xMax - xMin + 1, yMax - yMin + 1};
  }

  XMC_INLINE bool nextLine(int *px, int *py, int *pw) {
    do {
      if (nextCrossIndex < numCrosses - 1) {
        int x0 = std::max(xMin, xCrosses[nextCrossIndex++]);
        int x1 = std::min(xMax, xCrosses[nextCrossIndex++]);
        int w = x1 - x0;
        if (w <= 0) continue;
        *px = x0;
        *py = y;
        *pw = w;
        return true;
      } else {
        if (++y > yMax) return false;
        numCrosses = 0;
        for (int i = 0; i < numVertices; i++) {
          const vec2i &v1 = vertices[i];
          const vec2i &v2 = vertices[(i + 1) % numVertices];
          if ((v1.y <= y && v2.y > y) || (v1.y > y && v2.y <= y)) {
            int x = v1.x + (y - v1.y) * (v2.x - v1.x) / (v2.y - v1.y);
            xCrosses[numCrosses++] = x;
            if (numCrosses >= MAX_CROSSES) {
              // too many crosses, just give up and fill the whole line
              numCrosses = numCrosses / 2 * 2;  // make it even
              break;
            }
          }
        }
        for (int i = 0; i < numCrosses - 1; i++) {
          for (int j = 0; j < numCrosses - 1 - i; j++) {
            if (xCrosses[j] > xCrosses[j + 1]) {
              std::swap(xCrosses[j], xCrosses[j + 1]);
            }
          }
        }
        nextCrossIndex = 0;
      }
    } while (false);
    return true;
  }
};

}  // namespace xmc

#endif
