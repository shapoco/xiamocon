#ifndef XMC_GEO_RECT_HPP
#define XMC_GEO_RECT_HPP

#include "xmc/geo/geo_common.hpp"
namespace xmc {

/** Rectangle structure */
struct Rect {
  /** The x-coordinate of the top-left corner of the rectangle */
  int x;
  /** The y-coordinate of the top-left corner of the rectangle */
  int y;
  /** The width of the rectangle */
  int width;
  /** The height of the rectangle */
  int height;

  /** Check if a point is inside the rectangle */
  inline bool contains(int px, int py) const {
    return px >= x && px < x + width && py >= y && py < y + height;
  }

  /** Get the x-coordinate of the right edge of the rectangle */
  inline int right() const { return x + width; }

  /** Get the y-coordinate of the bottom edge of the rectangle */
  inline int bottom() const { return y + height; }

  /** Normalize the rectangle so that width and height are non-negative */
  inline void normalizeInPlace() {
    if (width < 0) {
      x += width;
      width = -width;
    }
    if (height < 0) {
      y += height;
      height = -height;
    }
  }

  /** Get the intersection of this rectangle with another rectangle */
  inline Rect intersect(const Rect& other) const {
    Rect result;
    result.x = x > other.x ? x : other.x;
    result.y = y > other.y ? y : other.y;
    int r = right() < other.right() ? right() : other.right();
    int b = bottom() < other.bottom() ? bottom() : other.bottom();
    result.width = r - result.x;
    result.height = b - result.y;
    if (result.width < 0) {
      result.width = 0;
    }
    if (result.height < 0) {
      result.height = 0;
    }
    return result;
  }
};
}  // namespace xmc

#endif
