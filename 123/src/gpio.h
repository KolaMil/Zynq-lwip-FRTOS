#ifndef __GPIO_H_
#define __GPIO_H_

#include "xgpiops.h"

XGpioPs  psGpioInstancePtr;
XGpioPs_Config* GpioConfigPtr;
int xStatus;

void init_gpio(void);
void vParTestInitialise(void);

#endif
