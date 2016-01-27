#include <stdlib.h>
#include <stdio.h>
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "gui/gui.h"
#include "macros.h"

// private functions

static void drawElementLabel(GUIElement *e);

// externally defined structures & functions

extern LTDC_HandleTypeDef hLtdcHandler;
extern void LL_ConvertLineToARGB8888(void * pSrc, void *pDst, uint32_t xSize,
		uint32_t ColorMode);

// push button functions

void handlePushButton(GUIElement *bt, GUITouchState *touch) {
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
		if (bt->callback != NULL) {
			bt->callback(bt);
		}
	}
}

void renderPushButton(GUIElement *bt) {
	if (bt->state & GUI_DIRTY) {
		SpriteSheet *sprite = bt->sprite;
		uint8_t state = bt->state & GUI_ONOFF_MASK;
		uint8_t id = state == GUI_ON ? 1 : 0;
		drawSprite(bt->x, bt->y, id, sprite);
		drawElementLabel(bt);
		// clear dirty flag
		bt->state &= ~((uint16_t) GUI_DIRTY);
	}
}

// dial button functions

void handleDialButton(GUIElement *bt, GUITouchState *touch) {
	if (touch->touchDetected) {
		// touch detected...
		uint16_t x = touch->touchX[0];
		uint16_t y = touch->touchY[0];
		DialButtonState *db = (DialButtonState *) bt->userData;
		if (bt->state == GUI_HOVER) {
			//int16_t dx = (x - db->startX);
			//int16_t dy = (y - db->startY);
			//int16_t delta = abs(dx) > abs(dy) ? dx : dy;
			int16_t delta = (x - db->startX);
			float newVal = db->startValue + db->sensitivity * delta;
			db->value = CLAMP(newVal, 0.0f, 1.0f);
			bt->state |= GUI_DIRTY;
			if (bt->callback != NULL) {
				bt->callback(bt);
			}
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

void renderDialButton(GUIElement *bt) {
	if (bt->state & GUI_DIRTY) {
		SpriteSheet *sprite = bt->sprite;
		DialButtonState *db = (DialButtonState *) bt->userData;
		uint8_t id = (uint8_t) (db->value * (float) (sprite->numSprites - 1));
		drawSprite(bt->x, bt->y, id, sprite);
		drawElementLabel(bt);
		bt->state &= ~((uint16_t) GUI_DIRTY);
	}
}

// common functionality

static void drawElementLabel(GUIElement *e) {
	if (e->label != NULL) {
		SpriteSheet *sprite = e->sprite;
		BSP_LCD_DisplayStringAt(e->x, e->y + sprite->spriteHeight + 4,
				(uint8_t*) e->label, LEFT_MODE);
	}
}

static uint8_t colorModeStrides[] = { 4, 3, 2, 2, 2 };

void drawSprite(uint16_t x, uint16_t y, uint8_t id, SpriteSheet *sprite) {
	uint32_t lcdWidth = BSP_LCD_GetXSize();
	uint32_t address = hLtdcHandler.LayerCfg[LTDC_ACTIVE_LAYER].FBStartAdress
			+ (((lcdWidth * y) + x) << 2);
	uint16_t width = sprite->spriteWidth;
	uint16_t height = sprite->spriteHeight;
	uint8_t *pixels = (uint8_t *) sprite->pixels;
	uint32_t stride = colorModeStrides[sprite->format];
	pixels += (id * width * height) * stride;
	stride *= width;
	lcdWidth <<= 2;
	while (--height) {
		LL_ConvertLineToARGB8888(pixels, (uint32_t *) address, width,
				sprite->format);
		address += lcdWidth;
		pixels += stride;
	}
}

void drawBitmapRaw(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
		uint8_t *pixels, uint32_t colorMode) {
	uint32_t lcdWidth = BSP_LCD_GetXSize();
	uint32_t address = hLtdcHandler.LayerCfg[LTDC_ACTIVE_LAYER].FBStartAdress
			+ (((lcdWidth * y) + x) << 2);
	uint16_t stride = width * colorModeStrides[colorMode];
	lcdWidth <<= 2;
	while (--height) {
		LL_ConvertLineToARGB8888(pixels, (uint32_t *) address, width,
				colorMode);
		address += lcdWidth;
		pixels += stride;
	}
}

// GUI element constructors

GUIElement *guiElement(uint8_t id, char *label, uint16_t x, uint16_t y,
		SpriteSheet *sprite, GUICallback cb) {
	GUIElement *e = (GUIElement *) calloc(1, sizeof(GUIElement));
	e->id = id;
	e->x = x;
	e->y = y;
	e->width = sprite->spriteWidth;
	e->height = sprite->spriteHeight;
	e->sprite = sprite;
	e->label = label;
	e->callback = cb;
	e->state = GUI_OFF | GUI_DIRTY;
	return e;
}

GUIElement *guiDialButton(uint8_t id, char *label, uint16_t x, uint16_t y,
		float val, float sens, SpriteSheet *sprite, GUICallback cb) {
	GUIElement *e = guiElement(id, label, x, y, sprite, cb);
	DialButtonState *db = (DialButtonState *) calloc(1,
			sizeof(DialButtonState));
	e->handler = handleDialButton;
	e->render = renderDialButton;
	e->userData = db;
	db->value = val;
	db->sensitivity = sens;
	return e;
}

GUIElement *guiPushButton(uint8_t id, char *label, uint16_t x, uint16_t y,
		float val, SpriteSheet *sprite, GUICallback cb) {
	GUIElement *e = guiElement(id, label, x, y, sprite, cb);
	PushButtonState *pb = (PushButtonState *) calloc(1,
			sizeof(PushButtonState));
	e->handler = handlePushButton;
	e->render = renderPushButton;
	e->userData = pb;
	pb->value = val;
	return e;
}

GUI *initGUI(uint8_t num, sFONT *font, uint32_t bgCol, uint32_t textCol) {
	GUI *gui = (GUI *) calloc(1, sizeof(GUI));
	gui->items = (GUIElement **) calloc(num, sizeof(GUIElement *));
	gui->numItems = num;
	gui->font = font;
	gui->bgColor = bgCol;
	gui->textColor = textCol;
	return gui;
}

void guiForceRedraw(GUI *gui) {
	for (uint8_t i = 0; i < gui->numItems; i++) {
		gui->items[i]->state |= GUI_DIRTY;
	}
}

void guiUpdate(GUI *gui, GUITouchState *touch) {
	BSP_LCD_SetFont(gui->font);
	BSP_LCD_SetBackColor(gui->bgColor);
	BSP_LCD_SetTextColor(gui->textColor);
	for (uint8_t i = 0; i < gui->numItems; i++) {
		GUIElement *e = gui->items[i];
		e->handler(e, touch);
		e->render(e);
	}
}

void guiUpdateTouch(TS_StateTypeDef *raw, GUITouchState *touch) {
	touch->touchDetected = raw->touchDetected;
	touch->touchX[0] = raw->touchX[0];
	touch->touchY[0] = raw->touchY[0];
}
