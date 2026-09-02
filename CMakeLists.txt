cmake_minimum_required(VERSION 3.20)
project(zynqRTOS C ASM)

set(BSP_DIR "${CMAKE_SOURCE_DIR}/zynqRTOS_bsp/ps7_cortexa9_0" CACHE PATH "BSP root")

add_subdirectory(src)