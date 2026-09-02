/*
 ============================================================================
 Name        : newlib_compat.c
 Author      :
 Version     :
 Description : Bridge between newlib from Xilinx SDK and newlib from gcc-arm-none-eabi
 ============================================================================
 */

#include <ctype.h>

// Здесь это макрос, без #undef он съест сигнатуру ниже
#undef __locale_ctype_ptr

extern const char _ctype_[];

/*-----------------------------------------------------------*/
// Библиотеки BSP собраны компилятором Xilinx, где макросы ctype.h зовут эту
// функцию. В gcc-arm-none-eabi её нет ни в одной мультибиблиотеке - отсюда
// undefined reference при линковке. Возвращаемое значение задано самим
// ctype.h: __locale_ctype_ptr() == _ctype_
const char *__locale_ctype_ptr(void)
{
	return _ctype_;
}
