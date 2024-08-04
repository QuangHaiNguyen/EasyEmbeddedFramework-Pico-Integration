cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH ${CMAKE_SOURCE_DIR}/lib/pico-sdk)
set(FREERTOS_KERNEL_PATH ${CMAKE_SOURCE_DIR}/lib/FreeRTOS-Kernel)

include(${CMAKE_SOURCE_DIR}/configs/features.cmake)
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)


project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(lib/EasyEmbeddedFramework)

add_executable(main
    src/main.c
)

target_include_directories(main
    PUBLIC
    ${CMAKE_SOURCE_DIR}/configs
)

pico_enable_stdio_usb(main 1)
pico_add_extra_outputs(main)

target_link_libraries(main
PUBLIC
    pico_stdlib
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    easy_embedded_lib
    )