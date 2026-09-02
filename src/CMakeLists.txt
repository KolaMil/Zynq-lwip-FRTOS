# Автоматический сбор всех .c из src/
# CONFIGURE_DEPENDS: при добавлении/удалении .c CMake переконфигурируется при сборке
file(GLOB APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(src ${APP_SOURCES})

target_include_directories(src PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${BSP_DIR}/include"
)

target_compile_options(src PRIVATE
    -mcpu=cortex-a9
    -mfpu=vfpv3
    -mfloat-abi=hard
    -Wall
    $<$<CONFIG:Debug>:-g3 -O0>
    $<$<CONFIG:Release>:-O2>
)

# Импорт готовых библиотек BSP
add_library(bsp_xil STATIC IMPORTED GLOBAL)
set_target_properties(bsp_xil PROPERTIES
    IMPORTED_LOCATION "${BSP_DIR}/lib/libxil.a"
)

add_library(bsp_lwip STATIC IMPORTED GLOBAL)
set_target_properties(bsp_lwip PROPERTIES
    IMPORTED_LOCATION "${BSP_DIR}/lib/liblwip4.a"
)

add_library(bsp_freertos STATIC IMPORTED GLOBAL)
set_target_properties(bsp_freertos PROPERTIES
    IMPORTED_LOCATION "${BSP_DIR}/lib/libfreertos.a"
)

# Линковка как в SDK
target_link_options(src PRIVATE
    -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard
    -Wl,-build-id=none
    -specs=${CMAKE_CURRENT_SOURCE_DIR}/src/Xilinx.spec
    -Wl,-Map=$<TARGET_FILE_DIR:src>/output.map
    -T ${CMAKE_CURRENT_SOURCE_DIR}/src/lscript.ld
    -L${BSP_DIR}/lib
    -Wl,--start-group
    -lxil -llwip4 -lgcc -lc
    -Wl,--end-group
)

target_link_libraries(src PRIVATE bsp_xil bsp_lwip bsp_freertos)

# Постобработка: размер и .bin для прошивки
add_custom_command(TARGET src POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:src>
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:src>
            $<TARGET_FILE_DIR:src>/src.bin
    COMMENT "Size + generate .bin"
)