#include <stdlib.h>
#include <stdio.h>
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "gui/gui.h"
#include "macros.h"

static uint32_t guiButtonColors[] = { 0xff000000, 0xffffff00 };

// externally defined structures & functions

extern LTDC_HandleTypeDef hLtdcHandler;
extern void LL_ConvertLineToARGB8888(void * pSrc, void *pDst, uint32_t xSize,
		uint32_t ColorMode);

void handlePushButton(GUIElement *bt, TS_StateTypeDef *touch) {
	if (touch->touchDetected) {
		// touch detected...
		uint16_t x = touch->touchX[0];
		uint16_t y = touch->touchY[0];
		if (x >= bt->x && x < bt->x + bt->width && y >= bt->y
				&& y < bt->y + bt->height) {
			switch (bt->state) {
			case GUI_OFF:
				bt->state |= GUI_HOVER | GUI_DIRTY;
				break;
			case GUI_ON:
				bt->state |= GUI_HOVER | GUI_DIRTY;
				break;
			default:
				break;
			}
		}
	} else if (bt->state & GUI_HOVER) {
		// clear hover flag
		bt->state &= ~((uint16_t) GUI_HOVER);
		// mark dirty (force redraw)
		bt->state |= GUI_DIRTY;
		// invert on/off bitmask
		bt->state ^= GUI_ONOFF_MASK;
	}
}

void handleDialButton(GUIElement *bt, TS_StateTypeDef *touch) {
	if (touch->touchDetected) {
		// touch detected...
		uint16_t x = touch->touchX[0];
		uint16_t y = touch->touchY[0];
		DialButtonState *db = (DialButtonState *) bt->userData;
		if (bt->state == GUI_HOVER) {
			float newVal = db->startValue + db->sensitivity * (x - db->startX);
			db->value = CLAMP(newVal, 0.0f, 1.0f);
			bt->state |= GUI_DIRTY;
		} else if (x >= bt->x && x < bt->x + bt->width && y >= bt->y
				&& y < bt->y + bt->height) {
			bt->state = GUI_HOVER;
			db->startX = x;
			db->startY = y;
			db->startValue = db->value;
		}
	} else if (bt->state == GUI_HOVER) {
		bt->state = GUI_OFF | GUI_DIRTY;
	}
}

void renderPushButton(GUIElement *bt) {
	if (bt->state & GUI_DIRTY) {
		SpriteSheet *sprite = bt->sprite;
		BSP_LCD_SetTextColor(
				bt->state & GUI_HOVER ?
						0xffff00ff :
						guiButtonColors[(bt->state & GUI_ONOFF_MASK) - 1]);
		BSP_LCD_FillRect(bt->x, bt->y, bt->width, bt->height);
		drawSprite(bt->x, bt->y, 0, sprite);
		BSP_LCD_SetTextColor(UI_TEXT_COLOR);
		BSP_LCD_DisplayStringAt(bt->x,
				bt->y + sprite->spriteHeight + 4, (uint8_t*) bt->label,
				LEFT_MODE);
		// clear dirty flag
		bt->state &= ~((uint16_t) GUI_DIRTY);
	}
}

void renderDialButton(GUIElement *bt) {
	if (bt->state & GUI_DIRTY) {
		SpriteSheet *sprite = bt->sprite;
		DialButtonState *db = (DialButtonState *) bt->userData;
		uint8_t id = (uint8_t) (db->value * (float) (sprite->numSprites - 1));
		drawSprite(bt->x, bt->y, id, sprite);
		BSP_LCD_SetTextColor(UI_TEXT_COLOR);
		BSP_LCD_DisplayStringAt(bt->x,
				bt->y + sprite->spriteHeight + 4, (uint8_t*) bt->label,
				LEFT_MODE);
		bt->state &= ~((uint16_t) GUI_DIRTY);
	}
}

void drawSprite(uint16_t x, uint16_t y, uint8_t id, SpriteSheet *sprite) {
	uint32_t lcdWidth = BSP_LCD_GetXSize();
	uint32_t address = hLtdcHandler.LayerCfg[LTDC_ACTIVE_LAYER].FBStartAdress
			+ (((lcdWidth * y) + x) << 2);
	uint16_t width = sprite->spriteWidth;
	uint16_t height = sprite->spriteHeight;
	uint8_t *pixels = (uint8_t *) sprite->pixels;
	pixels += (id * width * height) << 2;
	uint16_t stride = width << 2;
	lcdWidth <<= 2;
	while (--height) {
		LL_ConvertLineToARGB8888((uint32_t *) pixels, (uint32_t *) address,
				width,
				CM_ARGB8888);
		address += lcdWidth;
		pixels += stride;
	}
}

void drawBitmapRaw(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
		uint8_t *pixels) {
	uint32_t lcdWidth = BSP_LCD_GetXSize();
	uint32_t address = hLtdcHandler.LayerCfg[LTDC_ACTIVE_LAYER].FBStartAdress
			+ (((lcdWidth * y) + x) << 2);
	uint16_t stride = width << 2;
	lcdWidth <<= 2;
	while (--height) {
		LL_ConvertLineToARGB8888((uint32_t *) pixels, (uint32_t *) address,
				width,
				CM_ARGB8888);
		address += lcdWidth;
		pixels += stride;
	}
}

GUIElement *guiDialButton(uint8_t id, char *label, uint16_t x, uint16_t y,
		float val, float sens, SpriteSheet *sprite) {
	GUIElement *e = (GUIElement *) calloc(1, sizeof(GUIElement));
	DialButtonState *db = (DialButtonState *) calloc(1,
			sizeof(DialButtonState));
	e->id = id;
	e->x = x;
	e->y = y;
	e->width = sprite->spriteWidth;
	e->height = sprite->spriteHeight;
	e->sprite = sprite;
	e->label = label;
	e->state = GUI_OFF | GUI_DIRTY;
	//e->handler = handlePushButton;
	//e->render = renderPushButton;
	e->handler = handleDialButton;
	e->render = renderDialButton;
	e->userData = db;
	db->value = val;
	db->sensitivity = sens;
	return e;
}
