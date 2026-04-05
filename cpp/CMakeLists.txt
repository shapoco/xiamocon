set(LIB_NAME libxmc)
set(PFM_DECL_SRC_DIR platform/decl/src)
set(PFM_DECL_INC_DIR platform/decl/include)
set(PFM_IMPL_SRC_DIR platform/impl/rp2350_pico_sdk/src)
set(PFM_IMPL_INC_DIR platform/impl/rp2350_pico_sdk/include)
set(LIB_SRC_DIR library/src)
set(LIB_INC_DIR library/include)
set(LIB_EXT_DIR library/ext)

include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(${LIB_NAME} C CXX ASM)

file(GLOB CPP_FILES
    ${PFM_DECL_SRC_DIR}/*.cpp
    ${PFM_DECL_SRC_DIR}/*.c
    ${PFM_IMPL_SRC_DIR}/*.cpp
    ${PFM_IMPL_SRC_DIR}/*.c
    ${LIB_SRC_DIR}/*.cpp
    ${LIB_SRC_DIR}/*.c
    ${LIB_SRC_DIR}/**/*.cpp
    ${LIB_SRC_DIR}/**/*.c
    ${LIB_EXT_DIR}/**/*.cpp
    ${LIB_EXT_DIR}/**/*.c
)

add_library(${LIB_NAME} STATIC
    ${CPP_FILES}
)

target_compile_options(${LIB_NAME} PUBLIC -O2 -Wall)
target_compile_features(${LIB_NAME} PUBLIC cxx_std_17)

target_include_directories(${LIB_NAME} PUBLIC
    ${PFM_DECL_INC_DIR}
    ${PFM_IMPL_INC_DIR}
    ${LIB_INC_DIR}
    ${LIB_EXT_DIR}/fatfs
    ${LIB_EXT_DIR}/tjpgd
)

target_link_libraries(
    ${LIB_NAME}
    pico_stdlib
    pico_multicore
    pico_stdio_usb
    hardware_dma
    hardware_spi
    hardware_i2c
    hardware_pwm
    hardware_pio
    hardware_powman
    hardware_vreg
    hardware_watchdog
    hardware_sleep
)
