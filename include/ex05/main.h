#ifndef __EX05_MAIN_H
#define __EX05_MAIN_H

#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "stm32f401_discovery.h"
#include "stm32f401_discovery_audio.h"
#include "stm32f401_discovery_accelerometer.h"
#include "stm32f4xx_it.h"
#include "clockconfig.h"
#include "led.h"
#include "types.h"
#include "waveplayer.h"
#include "waverecorder.h"
#include "ff.h"    
#include "ff_gen_drv.h"
#include "usbh_diskio.h"

/* You can change the Wave file name as you need, but do not exceed 11 characters */
#define WAVE_NAME "0:sample.wav"
#define REC_WAVE_NAME "0:record.wav"
  
//#define MEMS_LIS3DSH 0x3F
//#define MEMS_LIS302DL 0x3B
                                                                                    
#endif
