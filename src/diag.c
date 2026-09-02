/*
 ============================================================================
 Name        : diag.c
 Author      :
 Version     :
 Description : FreeRTOS Fault Handlers
 ============================================================================
 */

#include "FreeRTOS.h"
#include "task.h"

extern void xil_printf(const char *fmt, ...);

// В libfreertos.a оба хука - пустые weak-заглушки. Из-за этого нехватка кучи
// и переполнение стека проходят молча: xTaskCreate возвращает pdFAIL, результат
// нигде не проверяется, задача просто не создаётся, прошивка работает дальше.

/*-----------------------------------------------------------*/
void vApplicationMallocFailedHook(void)
{
	// Куча 100000 байт вкомпилирована в libfreertos.a, через
	// configTOTAL_HEAP_SIZE её не увеличить - только урезать стеки задач
	xil_printf("\r\n[FATAL] FreeRTOS heap exhausted: free %u, min ever %u\r\n",
	           (unsigned)xPortGetFreeHeapSize(),
	           (unsigned)xPortGetMinimumEverFreeHeapSize());
	for (;;) { }
}

/*-----------------------------------------------------------*/
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	(void)xTask;
	xil_printf("\r\n[FATAL] stack overflow in task \"%s\"\r\n",
	           pcTaskName ? pcTaskName : "?");
	for (;;) { }
}
