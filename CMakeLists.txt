cmake_minimum_required(VERSION 3.27)

project(xilinx_mcu C CXX ASM)
#include(arm-none-eabi-gcc-toolchain.cmake)

set(CMAKE_CXX_STANDARD 14)
set(TOOLS_DIR "${CMAKE_SOURCE_DIR}/tools")

set(BUILD_OUTPUT_DIR "$<TARGET_FILE_DIR:xilinx_mcu.elf>")


#add_compile_definitions(ESC_DEBUG)


add_executable(xilinx_mcu.elf
        src/main.c
        src/platform.c
        src/tcp_stack.c
        src/ttc_timer.c
        src/udp_stack.c
        src/iwip.c
        src/cpu1_init.c
        src/asterix_parse.c
        src/sys_calls.c
        src/auto_gain_control.c
        src/pl_interface.c
        src/pl_interface.h
)

target_include_directories(xilinx_mcu.elf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/ps7_cortexa9_0/include)
#${CMAKE_CURRENT_SOURCE_DIR}/external/ps7_cortexa9_0/libsrc
#target_include_directories(xilinx_mcu.elf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/ps7_cortexa9_0/libsrc/lwip211_v1-0)

target_link_directories(xilinx_mcu.elf PRIVATE external/ps7_cortexa9_0/lib)
target_compile_options(xilinx_mcu.elf PRIVATE
        -Wall -O0 -g3 -c -fmessage-length=0
        -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -v
)

target_link_libraries(xilinx_mcu.elf PUBLIC
        liblwip4.a
        libfreertos.a
        libxil.a)

target_link_options(xilinx_mcu.elf PRIVATE
        -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -Wl,-build-id=none
        -specs=${CMAKE_SOURCE_DIR}/linker/Xilinx.spec -Wl,-T -Wl,${CMAKE_SOURCE_DIR}/linker/lscript.ld
        -Wl,--start-group,-lxil,-lgcc,-lc,--end-group
)

file(GLOB TOOLS_FILES "${TOOLS_DIR}/*")