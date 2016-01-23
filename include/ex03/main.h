#pragma once
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "ex03/stm32f7xx_it.h"
#include "clockconfig.h"
#include <stdio.h>

#define RGB565_BYTE_PER_PIXEL     2
#define ARBG8888_BYTE_PER_PIXEL   4
#define LCD_FRAME_BUFFER          SDRAM_DEVICE_ADDR

typedef void (*DemoFn)(void);

//void BSP_LCD_DMA2D_IRQHandler(void);
