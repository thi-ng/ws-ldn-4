#pragma once

#include "stm32f7xx_hal.h"
#include "stm32746g_discovery.h"
#include "clockconfig.h"

#define TIMx              TIM3
#define TIMx_CLK_ENABLE() __HAL_RCC_TIM3_CLK_ENABLE()
#define TIMx_IRQn         TIM3_IRQn
#define TIMx_IRQHandler   TIM3_IRQHandler

