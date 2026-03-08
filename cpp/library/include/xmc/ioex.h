/**
 * @file ioex.h
 * @brief IO Expander interface for XMC library
 */

#ifndef XMC_IOEX_H
#define XMC_IOEX_H

#include "xmc/xmc_common.h"

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * IO Expander pin definitions. These correspond to the pins on the IO expander
 * chip.
 */
typedef enum {
  /**
   * User-defined pin 0
   * @details This pin is connected to the expansion board. It is not used in
   * the Xiamocon main board.
   */
  XMC_IOEX_PIN_USER_0 = 0,
  /**
   * User-defined pin 1
   * @details This pin is connected to the expansion board. It is not used in
   * the Xiamocon main board.
   */
  XMC_IOEX_PIN_USER_1 = 1,

  /**
   * User-defined pin 2
   * @details This pin is connected to the expansion board. It is not used in
   * the Xiamocon main board.
   */
  XMC_IOEX_PIN_USER_2 = 2,

  /**
   * User-defined pin 3
   * @details This pin is connected to the expansion board. It is not used in
   * the Xiamocon main board.
   */
  XMC_IOEX_PIN_USER_3 = 3,

  /**
   * Low-active signal that controls power to various devices on the Xiamocon
   * board. Normally controlled by the system API.
   */
  XMC_IOEX_PIN_PERI_EN = 4,

  /**
   * Low-active signal that controls hardware reset of the display.
   * Normally controlled by the display API.
   */
  XMC_IOEX_PIN_DISPLAY_RESET = 5,

  /**
   * High-active signal that controls speaker mute. Normally controlled by the
   * audio API.
   */
  XMC_IOEX_PIN_SPEAKER_MUTE = 6,

  /**
   * Low-active signal that receives the pressed state of the function button.
   * Normally readed by the input API.
   */
  XMC_IOEX_PIN_BTN_FUNC = 7,

  /**
   * Low-active signals that receive the pressed state of the A button.
   * Normally readed by the input API.
   */
  XMC_IOEX_PIN_BTN_A = 8,

  /**
   * Low-active signals that receive the pressed state of the B button.
   * Normally readed by the input API.
   */
  XMC_IOEX_PIN_BTN_B = 9,

  /**
   * Low-active signals that receive the pressed state of the X button.
   * Normally readed by the input API.
   */
  XMC_IOEX_PIN_BTN_X = 10,

  /**
   * Low-active signals that receive the pressed state of the Y button.
   * Normally readed by the input API.
   */
  XMC_IOEX_PIN_BTN_Y = 11,

  /**
   * Low-active signals that receive the pressed state of the Up button.
   * Normally readed by the input API.
   */
  XMC_IOEX_PIN_BTN_UP = 12,

  /**
   * Low-active signals that receive the pressed state of the Down button.
   * Normally readed by the input API.
   */
  XMC_IOEX_PIN_BTN_DOWN = 13,

  /**
   * Low-active signals that receive the pressed state of the Left button.
   * Normally readed by the input API.
   */
  XMC_IOEX_PIN_BTN_LEFT = 14,

  /**
   * Low-active signals that receive the pressed state of the Right button.
   * Normally readed by the input API.
   */
  XMC_IOEX_PIN_BTN_RIGHT = 15,
} xmc_ioex_pin_t;

static const xmc_ioex_pin_t XMC_IOEX_GAME_BUTTON_PINS[] = {
    XMC_IOEX_PIN_BTN_A,    XMC_IOEX_PIN_BTN_B,     XMC_IOEX_PIN_BTN_X,
    XMC_IOEX_PIN_BTN_Y,    XMC_IOEX_PIN_BTN_UP,    XMC_IOEX_PIN_BTN_DOWN,
    XMC_IOEX_PIN_BTN_LEFT, XMC_IOEX_PIN_BTN_RIGHT,
};

/**
 * Initialize the IO expander. This function must be called before using any
 * other IO expander functions.
 * @return XMC_OK if the IO expander was successfully initialized.
 */
xmc_status_t xmc_ioex_init();

/**
 * Deinitialize the IO expander. This will reset the pin directions and output
 * values to their default states.
 * @return XMC_OK if the IO expander was successfully deinitialized.
 */
xmc_status_t xmc_ioex_deinit();

/**
 * Set the direction of the specified pins on the IO expander. The mask
 * parameter specifies which pins to modify, and the value parameter specifies
 * the direction for those pins (1 for output, 0 for input).
 * @param port The IO expander port number (0 or 1).
 * @param mask A bitmask specifying which pins to modify (1 for each pin to
 * modify).
 * @param value A bitmask specifying the direction for the modified pins (1 for
 * output, 0 for input).
 * @return XMC_OK if the pin directions were successfully set.
 */
xmc_status_t xmc_ioex_set_dir_masked(int port, uint8_t mask, uint8_t value);

/**
 * Write values to the specified pins on the IO expander. The mask parameter
 * specifies which pins to modify, and the value parameter specifies the output
 * value for those pins (1 for high, 0 for low).
 * @param port The IO expander port number (0 or 1).
 * @param mask A bitmask specifying which pins to modify (1 for each pin to
 * modify).
 * @param value A bitmask specifying the output value for the modified pins (1
 * for high, 0 for low).
 * @return XMC_OK if the pin values were successfully written.
 */
xmc_status_t xmc_ioex_write_masked(int port, uint8_t mask, uint8_t value);

/**
 * Read values from the specified pins on the IO expander. The mask parameter
 * specifies which pins to read, and the value parameter will be set to the
 * current state of those pins (1 for high, 0 for low).
 * @param port The IO expander port number (0 or 1).
 * @param mask A bitmask specifying which pins to read (1 for each pin to read).
 * @param value Pointer to a variable that will receive the state of the
 * specified pins (1 for high, 0 for low).
 * @return XMC_OK if the pin values were successfully read.
 */
xmc_status_t xmc_ioex_read_masked(int port, uint8_t mask, uint8_t *value);

/**
 * Convenience functions for setting the direction of a single pin. These
 * functions call xmc_ioex_set_dir_masked with the appropriate parameters for
 * the specified pin. The pin parameter specifies which pin to modify, and the
 * output parameter specifies the direction for that pin (true for output, false
 * for input).
 * @param pin The IO expander pin to modify.
 * @param output The direction for the specified pin (true for output, false for
 * input).
 * @return XMC_OK if the pin direction was successfully set.
 */
static inline xmc_status_t xmc_ioex_set_dir(xmc_ioex_pin_t pin, bool output) {
  int port = pin / 8;
  int bit = pin % 8;
  return xmc_ioex_set_dir_masked(port, 1 << bit, output ? (1 << bit) : 0);
}

/**
 * Convenience functions for writing a single pin. These functions call
 * xmc_ioex_write_masked with the appropriate parameters for the specified pin.
 * The pin parameter specifies which pin to modify, and the value parameter
 * specifies the output value for that pin (true for high, false for low).
 * @param pin The IO expander pin to modify.
 * @param value The output value for the specified pin (true for high, false for
 * low).
 * @return XMC_OK if the pin value was successfully written.
 */
static inline xmc_status_t xmc_ioex_write(xmc_ioex_pin_t pin, bool value) {
  int port = pin / 8;
  int bit = pin % 8;
  return xmc_ioex_write_masked(port, 1 << bit, value ? (1 << bit) : 0);
}

/**
 * Convenience functions for reading a single pin. These functions call
 * xmc_ioex_read_masked with the appropriate parameters for the specified pin.
 * The pin parameter specifies which pin to read, and the value parameter will
 * be set to the current state of that pin (true for high, false for low).
 * @param pin The IO expander pin to read.
 * @param value Pointer to a variable that will receive the state of the
 * specified pin (true for high, false for low).
 * @return XMC_OK if the pin value was successfully read.
 */
static inline xmc_status_t xmc_ioex_read(xmc_ioex_pin_t pin, bool *value) {
  int port = pin / 8;
  int bit = pin % 8;
  uint8_t temp;
  xmc_status_t status = xmc_ioex_read_masked(port, 1 << bit, &temp);
  if (status == XMC_OK) {
    *value = (temp != 0);
  }
  return status;
}

/**
 * Read the state of all pins on the IO expander. The value parameter will be
 * set to a 16-bit value where each bit represents the state of a pin (1 for
 * high, 0 for low). Bit 0 corresponds to pin 0, bit 1 corresponds to pin 1,
 * and so on up to bit 15 for pin 15.
 * @param value Pointer to a variable that will receive the state of all pins on
 * the IO expander (1 for high, 0 for low).
 * @return XMC_OK if the pin values were successfully read.
 */
xmc_status_t xmc_ioex_read_all(uint16_t *value);

#if defined(__cplusplus)
}
#endif

#endif
