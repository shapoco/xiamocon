#ifndef XMC_GFX_ATTRIBUTE_BUFFER_HPP
#define XMC_GFX_ATTRIBUTE_BUFFER_HPP

#include "xmc/geo.hpp"
#include "xmc/gfx2d/colorf.hpp"
#include "xmc/gfx3d/gfx3d_common.hpp"
#include "xmc/hw/heap.hpp"

#include <memory>

namespace xmc {

template <typename T>
class AttributeBufferClass {
 public:
  T *data;
  int size;

 protected:
  bool autoFree;

 public:
  AttributeBufferClass(T *data, int size, bool autoFree = false)
      : data(data), size(size), autoFree(autoFree) {}

  AttributeBufferClass(int size, XmcHeapCap caps)
      : data((T *)xmcMalloc(sizeof(T) * size, caps)),
        size(size),
        autoFree(true) {}

  ~AttributeBufferClass() {
    if (autoFree) {
      if (data) {
        xmcFree((void *)data);
        data = nullptr;
      }
    }
  }
};

using Vec2Buffer = std::shared_ptr<AttributeBufferClass<vec2>>;
static inline Vec2Buffer createVec2Buffer(int size,
                                          XmcHeapCap caps = XMC_HEAP_CAP_SPIRAM) {
  return std::make_shared<AttributeBufferClass<vec2>>(size, caps);
}
static inline Vec2Buffer createVec2Buffer(vec2 *data, int size,
                                          bool autoFree = false) {
  return std::make_shared<AttributeBufferClass<vec2>>(data, size, autoFree);
}

using Vec3Buffer = std::shared_ptr<AttributeBufferClass<vec3>>;
static inline Vec3Buffer createVec3Buffer(int size,
                                          XmcHeapCap caps = XMC_HEAP_CAP_SPIRAM) {
  return std::make_shared<AttributeBufferClass<vec3>>(size, caps);
}
static inline Vec3Buffer createVec3Buffer(vec3 *data, int size,
                                          bool autoFree = false) {
  return std::make_shared<AttributeBufferClass<vec3>>(data, size, autoFree);
}

using ColorBuffer = std::shared_ptr<AttributeBufferClass<colorf>>;
static inline ColorBuffer createColorBuffer(
    int size, XmcHeapCap caps = XMC_HEAP_CAP_SPIRAM) {
  return std::make_shared<AttributeBufferClass<colorf>>(size, caps);
}
static inline ColorBuffer createColorBuffer(colorf *data, int size,
                                            bool autoFree = false) {
  return std::make_shared<AttributeBufferClass<colorf>>(data, size, autoFree);
}

using IndexBuffer = std::shared_ptr<AttributeBufferClass<uint16_t>>;
static inline IndexBuffer createIndexBuffer(
    int size, XmcHeapCap caps = XMC_HEAP_CAP_SPIRAM) {
  return std::make_shared<AttributeBufferClass<uint16_t>>(size, caps);
}

static inline IndexBuffer createIndexBuffer(uint16_t *data, int size,
                                            bool autoFree = false) {
  return std::make_shared<AttributeBufferClass<uint16_t>>(data, size, autoFree);
}

}  // namespace xmc

#endif
