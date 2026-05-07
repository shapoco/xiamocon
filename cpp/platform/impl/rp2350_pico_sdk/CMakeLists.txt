set(LIB_NAME xmc_pfm)
set(PFM_DECL_SRC_DIR ../../decl/src)
set(PFM_DECL_INC_DIR ../../decl/include)
set(PFM_IMPL_SRC_DIR src)
set(PFM_IMPL_INC_DIR include)

project(${LIB_NAME} C CXX ASM)

file(GLOB CPP_FILES
    ${PFM_DECL_SRC_DIR}/*.cpp
    ${PFM_DECL_SRC_DIR}/*.c
    ${PFM_IMPL_SRC_DIR}/*.cpp
    ${PFM_IMPL_SRC_DIR}/*.c
)

add_library(${LIB_NAME} STATIC
    ${CPP_FILES}
)

target_compile_options(${LIB_NAME} PUBLIC -O2 -Wall)
target_compile_features(${LIB_NAME} PUBLIC cxx_std_17)

target_include_directories(${LIB_NAME} PUBLIC
    ${PFM_DECL_INC_DIR}
    ${PFM_IMPL_INC_DIR}
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
    hardware_adc
    hardware_powman
    hardware_vreg
    hardware_watchdog
    hardware_sleep
    hardware_flash
)
