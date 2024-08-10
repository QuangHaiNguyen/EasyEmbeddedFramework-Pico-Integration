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

# Config FreeRTOS
add_subdirectory(lib/EasyEmbeddedFramework)
set(FREERTOS_HEAP "4" CACHE INTERNAL "")
set(FREERTOS_PORT "GCC_RP2040" CACHE INTERNAL "")
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM
INTERFACE
    ${CMAKE_SOURCE_DIR}/configs
)
target_compile_definitions(freertos_config
INTERFACE
    projCOVERAGE_TEST=0
)
target_link_libraries(freertos_config
INTERFACE
    pico_stdlib
)
add_subdirectory(lib/FreeRTOS-Kernel)

# Config main
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
    easy_embedded_lib
    freertos_kernel
)
