/**
 * @file frame_buffer.hpp
 * @brief Frame buffer management for 2D graphics in XMC library
 */

#ifndef XMC_GFX_FRAME_BUFFER_HPP
#define XMC_GFX_FRAME_BUFFER_HPP

#include "xmc/gfx2d/graphics2d.hpp"
#include "xmc/gfx2d/sprite444.hpp"
#include "xmc/gfx2d/sprite565.hpp"

#include <memory>

namespace xmc {

enum class FrameBufferFlags : uint32_t {
  SHOW_STATUS_BAR = 1 << 1,
  SHOW_BATTERY_LEVEL = 1 << 2,
  SHOW_FPS = 1 << 3,
  SHOW_DEBUG_INFO = 1 << 4,
  DEFAULT = SHOW_STATUS_BAR | SHOW_BATTERY_LEVEL | SHOW_FPS,
};
XMC_ENUM_FLAGS(FrameBufferFlags, uint32_t)

class FrameBuffer {
 public:
  static constexpr int STATUS_BAR_HEIGHT = 10;

 private:
  const int numBuffers;
  FrameBufferFlags flags = FrameBufferFlags::DEFAULT;
  Sprite sprites[2];
  int frontIndex = 0;
  bool transferInProgress = false;

  uint64_t fpsLastUpdateUs = 0;
  uint32_t fpsFrameCount = 0;
  char fpsString[16] = {0};

  uint16_t lastVoltageMv = 0;
  char batString[16] = {0};

 public:
  FrameBuffer(PixelFormat fmt, bool doubleBuffered = false,
              int width = display::WIDTH, int height = display::HEIGHT);

  /**
   * @brief Check if the frame buffer is double buffered
   * @return True if double buffering is enabled, false otherwise
   */
  inline bool isDoubleBuffered() const { return numBuffers > 1; }

  /**
   * @brief Enable a specific flag for the frame buffer
   * @param flag The flag to enable
   */
  inline void enableFlag(FrameBufferFlags flag) { flags |= flag; }

  /**
   * @brief Disable a specific flag for the frame buffer
   * @param flag The flag to disable
   */
  inline void disableFlag(FrameBufferFlags flag) { flags &= ~flag; }

  /**
   * @brief Get the width of the frame buffer
   * @return The width of the frame buffer in pixels
   */
  inline int getWidth() const { return sprites[0]->width; }

  /**
   * @brief Get the height of the frame buffer
   * @return The height of the frame buffer in pixels
   */
  inline int getHeight() const { return sprites[0]->height; }

  /**
   * @brief Get the pixel format of the frame buffer
   * @return The pixel format of the frame buffer
   */
  inline PixelFormat getPixelFormat() const { return sprites[0]->format; }

  /**
   * @brief Get the front buffer sprite for rendering
   * @return A shared pointer to the front buffer sprite
   */
  inline Sprite getFrontBuffer() const {
    return sprites[frontIndex % numBuffers];
  }

  /**
   * @brief Get the back buffer sprite for rendering
   * @return A shared pointer to the back buffer sprite
   */
  inline Sprite getBackBuffer() const {
    return sprites[(frontIndex + 1) % numBuffers];
  }

  /**
   * @brief Create a Graphics2D object for rendering to the back buffer
   * @return A shared pointer to a Graphics2D object targeting the back buffer
   */
  inline Graphics2D createGraphics() const {
    return createGraphics2D(getBackBuffer());
  }

  /**
   * @brief Begin rendering to the frame buffer. If double buffering is disabled
   * and a transfer to display is in progress, this function will wait for the
   * transfer to complete before allowing rendering to begin.
   * @return An XmcStatus code indicating success or failure of the operation
   */
  XmcStatus beginRender();

  /**
   * @brief End rendering to the frame buffer. If transferToDisplay is true,
   * this function will initiate a transfer of the back buffer to the display.
   * If double buffering is enabled and a transfer is already in progress, this
   * function will wait for the current transfer to complete before starting a
   * new one.
   * @param transferToDisplay If true, the back buffer will be transferred to
   * the display after rendering is complete.
   * @return An XmcStatus code indicating success or failure of the operation
   */
  XmcStatus endRender(bool transferToDisplay = true);

  /**
   * @brief Render the status bar on the frame buffer. This function should be
   * called during the rendering process if the SHOW_STATUS_BAR flag is enabled.
   * It will draw the status bar at the top of the frame buffer, displaying
   * information such as battery level and FPS if the corresponding flags are
   * enabled.
   * @param gfx A Graphics2D object to use for rendering the status bar
   * @return An XmcStatus code indicating success or failure of the operation
   */
  XmcStatus renderStatusBar(Graphics2D &gfx);

  /**
   * @brief Render debug information on the frame buffer. This function should
   * be called during the rendering process if the SHOW_DEBUG_INFO flag is
   * enabled. It can be used to display various debug information such as memory
   * usage, performance metrics, or any other relevant data for debugging
   * purposes.
   * @param gfx A Graphics2D object to use for rendering the debug information
   * @return An XmcStatus code indicating success or failure of the operation
   */
  XmcStatus renderDebugBar(Graphics2D &gfx);
};

}  // namespace xmc

#endif
