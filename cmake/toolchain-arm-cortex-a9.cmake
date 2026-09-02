set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Путь к Xilinx GCC 2019.1 или свой arm-none-eabi-gcc
set(TOOLCHAIN_PREFIX "C:/Xilinx/SDK/2019.1/gnu/aarch32/nt/gcc-arm-none-eabi/bin/arm-none-eabi-")
# set(TOOLCHAIN_ROOT "C:/Xilinx/SDK/2019.1/gnu/aarch32/nt/gcc-arm-none-eabi")

set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc.exe)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++.exe)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc.exe)
set(CMAKE_AR           ${TOOLCHAIN_PREFIX}ar.exe)
set(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}objcopy.exe)
set(CMAKE_OBJDUMP      ${TOOLCHAIN_PREFIX}objdump.exe)
set(CMAKE_SIZE         ${TOOLCHAIN_PREFIX}size.exe)

set(CMAKE_C_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_DEBUG "" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "" CACHE STRING "" FORCE)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")