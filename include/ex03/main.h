#pragma once
#include <stdint.h>
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "ex03/stm32f7xx_it.h"
#include "gui/gui.h"
#include "clockconfig.h"

#define LCD_FRAME_BUFFER SDRAM_DEVICE_ADDR

typedef void (*DemoFn)(void);
