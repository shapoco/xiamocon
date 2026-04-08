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

class FrameBuffer {
 public:
  const int numBuffers;
  Sprite sprites[2];
  int frontIndex = 0;
  bool transferInProgress = false;

  FrameBuffer(PixelFormat fmt, bool doubleBuffered = false,
              int width = display::WIDTH, int height = display::HEIGHT);

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
};

}  // namespace xmc

#endif
