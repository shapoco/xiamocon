/**
 * @file multicore.hpp
 * @brief Multicore support.
 */

#ifndef XMC_MULTICORE_HPP
#define XMC_MULTICORE_HPP

#include "xmc/xmc_common.hpp"

namespace xmc {

/**
 * @brief Function type for tasks to run on the second core. The function should
 * return true to continue running, or false to stop the core.
 */
using Core1TaskFunc = bool (*)();

/**
 * @brief Start the second core with the given task function. The task function
 * will be called repeatedly until it returns false or the core is stopped.
 * @param task The task function to run on the second core.
 * @return XMC_OK on success, or an error code on failure.
 */
XmcStatus startCore1(Core1TaskFunc task);

/**
 * @brief Stop the second core. This will signal the core to stop and wait for
 * it to finish. If the core does not stop within the given timeout, it will be
 * forcefully reset.
 * @param timeoutMs The maximum time to wait for the core to stop, in
 * milliseconds.
 * @return XMC_OK on success, or an error code on failure.
 */
XmcStatus stopCore1(uint32_t timeoutMs = 1000);

}  // namespace xmc

#endif
