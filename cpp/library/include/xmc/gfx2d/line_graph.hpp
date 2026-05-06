#ifndef XMC_GFX2D_LINE_GRAPH_HPP
#define XMC_GFX2D_LINE_GRAPH_HPP

#include <memory>
#include "xmc/gfx2d/graphics2d.hpp"

namespace xmc {

template <typename TValue>
class LineGraphClass {
 public:
  const int capacity;

 private:
  TValue *data;
  TValue minValue;
  TValue maxValue;
  int readPos;
  int count;

 public:
  LineGraphClass(int capacity, XmcHeapCap heapCap = XMC_HEAP_CAP_SPIRAM)
      : capacity(capacity) {
    data = (TValue *)xmcMalloc(sizeof(TValue) * capacity, heapCap);
    readPos = 0;
    count = 0;
  }

  ~LineGraphClass() {
    if (data) {
      xmcFree(data);
      data = nullptr;
    }
  }

  void clear() {
    readPos = 0;
    count = 0;
  }

  void pushValue(TValue value) {
    if (!data) return;
    int writePos = (readPos + count) % capacity;
    data[writePos] = value;
    if (count < capacity) {
      count++;
    } else {
      readPos = (readPos + 1) % capacity;
    }
  }

  void setYRange(TValue min, TValue max) {
    minValue = min;
    maxValue = max;
  }

  void getYRange(TValue *min = nullptr, TValue *max = nullptr) {
    if (min) *min = minValue;
    if (max) *max = maxValue;
  }

  void render(Graphics2D &gfx, int x, int y, int w, int h, DevColor color) {
    if (!data || count < 2) return;
    TValue range = maxValue - minValue;
    if (range <= 0) return;
    float rangeInv = 1.0f / range;
    int prevX = x;
    int prevY = y + h - ((data[readPos] - minValue) * h * rangeInv);
    int stepFrac = w * 0x100 / capacity;
    for (int i = 1; i < count; i++) {
      int idx = (readPos + i) % capacity;
      int currX = x + ((i * stepFrac) >> 8);
      int currY = y + h - ((data[idx] - minValue) * h * rangeInv);
      gfx->drawLine(prevX, prevY, currX, currY, color);
      prevX = currX;
      prevY = currY;
    }
  }
};

using LineGraph = std::shared_ptr<LineGraphClass<uint16_t>>;

static inline LineGraph createLineGraph(
    int capacity, XmcHeapCap heapCap = XMC_HEAP_CAP_SPIRAM) {
  return std::make_shared<LineGraphClass<uint16_t>>(capacity, heapCap);
}

}  // namespace xmc

#endif
