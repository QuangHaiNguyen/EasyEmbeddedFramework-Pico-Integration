# Integrate EasyEmbedded framework to Pi Pico project

## Introduction

This project demonstrates how to integrate [EasyEmbedded framework](https://github.com/QuangHaiNguyen/EasyEmbeddedFramework) and FreeRTOS into a Pi Pico software project. The
demonstrated project can be configured to run two applications:
- Two FreeRTOS tasks printing messages on the terminal
- Using task worker, which is an implementation of task queue/dispatch thread
  pattern.

## Getting started

### Getting the external library
All of the external libraries (Pico SDK, FreeRTOS, EasyEmbedded framework) are
configured as submodules and they locate in *lib folder*. Executing the following
commands to download the libraries.

```shell
cd lib

git submodule init

git submodule update
```

### Configure the external library

Originally, after downloading the libraries, users should configure the libraries
according to the target system. However, this reporitory includes already the
configurations, so we just need to go through them and explain what each
configuration does.

The starting point of the configuration is the CMakeLists.txt in the root folder.

#### Pico SDK
According to the Pico SDK, to integrate the it into a Pi Pico own project,
the following steps must be done:
- Set the path PICO_SDK_PATH
- Include the cmake build script (pico_sdk_import.cmake)
- Set the standard output (we will use USB comport as debug output)
- Link the sdk to the targets

Those steps are translated to the following CMake commands:

```CMake
set(PICO_SDK_PATH ${CMAKE_SOURCE_DIR}/lib/pico-sdk)

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)


pico_enable_stdio_usb(main 1)
pico_add_extra_outputs(main)

target_link_libraries(main
PUBLIC
    pico_stdlib
    easy_embedded_lib
    freertos_kernel
)
```

*Note:* The commands above are just a collection of commands, they do not
reflect their order in the CMake file. Please consult the final
[CMake file](./CMakeLists.txt) for the final result.

#### FreeRTOS

To integrate FreeRTOS, things are slightly complex. According to FreeRTOS
getting started, to integrate FreeRTOS, we need to do the following steps:
- Set the path to FreeRTOS source
- Set the heap configuration
- Set the target (FreeRTOS port), provided that the port is available for that target
- Set the path to FreeRTOS import script for that target (FreeRTOS_Kernel_import.cmake)
- Provide a FreeRTOSConfigs.h file and link it to the build system
- Add FreeRTOS source files as subdirectory
- Link Pico SDK to FreeRTOS because the porting uses code from the Pico SDK
- Link the FreeRTOS to the project.

Those steps are translated to the following CMake commands:

```CMake
set(FREERTOS_KERNEL_PATH ${CMAKE_SOURCE_DIR}/lib/FreeRTOS-Kernel)

include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

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

target_link_libraries(main
PUBLIC
    pico_stdlib
    easy_embedded_lib
    freertos_kernel
)
```

*Note:* The commands above are just a collection of commands, they do not
reflect their order in the CMake file. Please consult the final
[CMake file](./CMakeLists.txt) for the final result.

#### EasyEmbedded framework

Integrate EasyEmbedded framework is relatively simple since most of the code
are hardware-independent. To integrate the framework, following steps are required:
- Provide an configuration file (features.cmake), which contains a list of
  activated component and link the configuration file to CMake build
- Add the source code as subdirectory
- Link the framework to the project

```CMake
include(${CMAKE_SOURCE_DIR}/configs/features.cmake)
add_subdirectory(lib/EasyEmbeddedFramework)

target_link_libraries(main
PUBLIC
    pico_stdlib
    easy_embedded_lib
    freertos_kernel
)
```

*Note:* The commands above are just a collection of commands, they do not
reflect their order in the CMake file. Please consult the final
[CMake file](./CMakeLists.txt) for the final result.

If EasyEmbedded framework uses task worker, it also requires FreeRTOS. As long
as FreeRTOS is configured as describe in the aboved section, things should be
fine. The rest is taken care internally.

## Caveat
If you build this project according to the steps in previous section, you will
encounter the error: "error: 'sio_hw' undeclared (first use in this function)".

This error is caused by the following line:

```c
sio_hw->fifo_wr = 0;
```

in the file FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/port.c. This line
is only needed when we use multicore. Therefore, we can comment this line out.

## Build and run application

Building the application is straight forward, you just need to follow these
steps (assume you are in the root folder):

```shell
mkdir build

cd build

cmake ..

cmake --build .
```

To flash the binary into the Pi Pico, please follow the
[Pi Pico Getting Started](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf).

That's it. You have finished integrating several external libraries into your
Pi Pico Project. Happy Coding!

## Reference

- [Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [FreeRTOS Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel)
- [EasyEmbedded framework](https://github.com/QuangHaiNguyen/EasyEmbeddedFramework)

