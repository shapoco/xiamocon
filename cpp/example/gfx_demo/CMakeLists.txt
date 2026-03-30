cmake_minimum_required(VERSION 3.12)

set(APP_NAME imu_demo)
set(XMC_PLATFORM_DIR ../../platform)
set(XMC_LIBRARY_DIR ../../library)
set(APP_INC_DIR ./include)
set(APP_SRC_DIR ./src)

include(../../pico_sdk_import.cmake)
include(../../pico_extras_import.cmake)

project(${APP_NAME} C CXX ASM)

pico_sdk_init()

add_executable(${APP_NAME})

target_compile_options(${APP_NAME} PUBLIC -O2 -Wall)

add_subdirectory(../../ libxmc)

file(GLOB CPP_FILES
    ${APP_SRC_DIR}/*.cpp
    ${APP_SRC_DIR}/*.c
)

target_sources(${APP_NAME} PRIVATE 
    ${CPP_FILES}
)

target_include_directories(${APP_NAME} PRIVATE
    ${APP_INC_DIR}
    ${XMC_PLATFORM_DIR}/common/include
    ${XMC_LIBRARY_DIR}/include
)

target_link_libraries(${APP_NAME} PRIVATE
    pico_stdlib
    pico_multicore
    hardware_dma
    hardware_spi
    hardware_i2c
    libxmc
)

pico_enable_stdio_usb(${APP_NAME} 0)
pico_enable_stdio_uart(${APP_NAME} 0)

pico_add_extra_outputs(${APP_NAME})