/*
 * rtos_config.c
 *
 * Created: 10/10/2022 17:38:19
 *  Author: sarah
 */ 

#define RTOS_CONFIG_H_

#include "asf.h"
#include "conf_board.h"

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	for (;;) {
	}
}

extern void vApplicationIdleHook(void) {
	pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
}

extern void vApplicationTickHook(void) {}

extern void vApplicationMallocFailedHook(void) {
	configASSERT((volatile void *)NULL);
}