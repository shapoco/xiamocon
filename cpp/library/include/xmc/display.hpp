/**
 * @file display.h
 * @brief Display interface for XMC library
 */

#ifndef XMC_DISPLAY_HPP
#define XMC_DISPLAY_HPP

#include "xmc/xmc_common.hpp"

#include <stddef.h>

namespace xmc::display {

/**
 * Display interface formats. This is used to specify the pixel format of the
 * display interface. The actual pixel format used by the display may be fixed,
 * but this allows the library to know how to format pixel data when writing to
 * the display.
 */
enum class InterfaceFormat {
  RGB444,
  RGB565,
  RGB666,
};

/** Display width in pixels */
static constexpr int WIDTH = 240;

/** Display height in pixels */
static constexpr int HEIGHT = 240;

/**
 * Initialize the display. This will set up the GPIO pins, reset the display,
 * and configure it for use. The format parameter specifies the pixel format of
 * the display interface, which determines how pixel data should be formatted
 * when writing to the display. The rotation parameter specifies the rotation of
 * the display.
 * @param format The pixel format of the display interface.
 * @param rotation The rotation of the display.
 * @return XMC_OK if the display was successfully initialized.
 */
XmcStatus init(InterfaceFormat format, int rotation);

/**
 * Deinitialize the display. This will reset the GPIO pins and put the display
 * into a low-power state.
 * @return XMC_OK if the display was successfully deinitialized.
 */
XmcStatus deinit();

/**
 * Get the current interface format of the display. This returns the pixel format
 * that was specified when the display was initialized.
 * @return The current interface format of the display.
 */
InterfaceFormat getInterfaceFormat();

/**
 * Clear the display to a specific color.
 * @param color The color to fill the display with, in the same format as the
 *              display interface.
 * @return XMC_OK if the display was successfully cleared.
 */
XmcStatus clear(uint32_t color);

/**
 * Fill a rectangle on the display with a specific color.
 * @param x The x-coordinate of the top-left corner of the rectangle.
 * @param y The y-coordinate of the top-left corner of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @param color The color to fill the rectangle with, in the same format as the
 *              display interface.
 * @return XMC_OK if the rectangle was successfully filled.
 */
XmcStatus fillRect(int x, int y, int width, int height, uint32_t color);

/**
 * Set the active window on the display for subsequent pixel writes. The window
 * is defined by its top-left corner (x, y) and its width and height.
 * @param x The x-coordinate of the top-left corner of the window.
 * @param y The y-coordinate of the top-left corner of the window.
 * @param width The width of the window.
 * @param height The height of the window.
 * @return XMC_OK if the window was successfully set.
 */
XmcStatus setWindow(int x, int y, int width, int height);

/**
 * Start writing pixel data to the display. This should be called after setting
 * the active window. The data parameter points to the pixel data to be written,
 * and num_bytes specifies the number of bytes to write. If repeated is true,
 * the same pixel data will be repeated until num_bytes bytes have been written.
 * This is useful for filling large areas with a single color.
 * @param data Pointer to the pixel data to be written to the display.
 * @param num_bytes The number of bytes to write from the data buffer.
 * @param repeated If true, the same pixel data will be repeated until num_bytes
 * bytes have been written.
 * @return XMC_OK if the pixel data was successfully written to the display.
 */
XmcStatus writePixelsStart(const void *data, uint32_t num_bytes, bool repeated);

/**
 * Complete a pixel write operation started by writePixelsStart.
 * This should be called after starting a pixel write operation to wait for it
 * to finish.
 * @return XMC_OK if the pixel write operation was successfully completed.
 */
XmcStatus writePixelsComplete();

/**
 * Write a command to the display. This is a low-level function that sends a
 * command byte followed by optional parameter bytes to the display.
 * Higher-level functions will typically use this function to implement specific
 * commands.
 * @param cmd The command byte to send to the display.
 * @param params Pointer to the parameter bytes to send after the command byte.
 * @param num_params The number of parameter bytes to send.
 * @return XMC_OK if the command was successfully written to the display.
 */
XmcStatus writeCommand(const uint8_t cmd, const uint8_t *params,
                       uint32_t num_params);

/**
 * Convenience functions for writing commands with a specific number of
 * parameters. These functions call writeCommand with the
 * appropriate parameters.
 */
static inline XmcStatus writeCommandNoParam(uint8_t cmd) {
  return writeCommand(cmd, NULL, 0);
}

/**
 * Convenience functions for writing commands with a specific number of
 * parameters. These functions call writeCommand with the
 * appropriate parameters.
 */
static inline XmcStatus writeCommand1Param(uint8_t cmd, uint8_t data0) {
  return writeCommand(cmd, &data0, 1);
}

/**
 * Convenience functions for writing commands with a specific number of
 * parameters. These functions call writeCommand with the
 * appropriate parameters.
 */
static inline XmcStatus writeCommand2Params(uint8_t cmd, uint8_t data0,
                                            uint8_t data1) {
  uint8_t params[2] = {data0, data1};
  return writeCommand(cmd, params, 2);
}

/**
 * Convenience functions for writing commands with a specific number of
 * parameters. These functions call writeCommand with the
 * appropriate parameters.
 */
static inline XmcStatus writeCommand3Params(uint8_t cmd, uint8_t data0,
                                            uint8_t data1, uint8_t data2) {
  uint8_t params[3] = {data0, data1, data2};
  return writeCommand(cmd, params, 3);
}

/**
 * Convenience functions for writing commands with a specific number of
 * parameters. These functions call writeCommand with the
 * appropriate parameters.
 */
static inline XmcStatus writeCommand4Params(uint8_t cmd, uint8_t data0,
                                            uint8_t data1, uint8_t data2,
                                            uint8_t data3) {
  uint8_t params[4] = {data0, data1, data2, data3};
  return writeCommand(cmd, params, 4);
}

}  // namespace xmc::display

#endif
