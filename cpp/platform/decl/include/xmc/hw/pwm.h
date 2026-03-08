/**
 * @file pwm.h
 * @brief Hardware PWM interface
 */

#ifndef XMC_HW_PWM_H
#define XMC_HW_PWM_H

#include "xmc/hw/dma.h"
#include "xmc/hw/hw_common.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
  int pin;
  uint32_t freq_hz;
  uint32_t period;
} xmc_pwm_config_t;

typedef struct {
  void *handle;
} xmc_pwm_inst_t;

/**
 * Initialize a PWM instance with the specified configuration. This will set up
 * the PWM hardware and configure the specified pin for PWM output.
 * @param inst Pointer to a PWM instance to initialize. The handle field will be
 * set by this function and should not be modified by the caller.
 * @param cfg Pointer to a configuration structure specifying the PWM
 * parameters.
 * @param actual_freq_hz Pointer to a float where the actual frequency will be
 * stored. This may differ slightly from the requested frequency due to hardware
 * limitations. Can be NULL if the actual frequency is not needed.
 * @return XMC_OK if the PWM instance was successfully initialized, or an
 * appropriate error code if initialization failed.
 */
xmc_status_t xmc_pwm_init(xmc_pwm_inst_t *inst, const xmc_pwm_config_t *cfg, float *actual_freq_hz);

/**
 * Deinitialize a PWM instance. This will disable the PWM output and release any
 * resources associated with the instance.
 * @param inst Pointer to the PWM instance to deinitialize. The handle field
 * will be set to NULL by this function.
 */
xmc_status_t xmc_pwm_deinit(xmc_pwm_inst_t *inst);

/**
 * Set the duty cycle of a PWM instance. The cycle parameter specifies the duty
 * cycle as a value between 0 and the period specified in the configuration. For
 * example, if the period is 1000 and the cycle is 500, the duty cycle will be
 * 50%.
 * @param inst Pointer to the PWM instance to modify.
 * @param cycle The duty cycle to set, as a value between 0 and the period
 * specified in the configuration.
 * @return XMC_OK if the duty cycle was successfully set, or an appropriate
 * error code if the operation failed.
 */
xmc_status_t xmc_pwm_set_duty_cycle(xmc_pwm_inst_t *inst, uint32_t cycle);

/**
 * Start a DMA transfer to update the PWM duty cycle. The cfg parameter
 * specifies the DMA configuration to use for the transfer.
 * @param inst Pointer to the PWM instance to modify.
 * @param cfg Pointer to the DMA configuration to use for the transfer.
 * @return XMC_OK if the DMA transfer was successfully started, or an
 * appropriate error code if the operation failed.
 */
xmc_status_t xmc_pwm_dma_write_start(xmc_pwm_inst_t *inst,
                                     const xmc_dma_config_t *cfg);

#if defined(__cplusplus)
}
#endif

#endif
