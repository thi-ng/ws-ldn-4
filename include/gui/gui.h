#pragma once

#include "stm32746g_discovery_lcd.h"

#define UI_BG_COLOR   ((uint32_t)0xff59626c)
#define UI_TEXT_COLOR ((uint32_t)0xffcccccc)
#define UI_FONT Font12
#define UI_SENSITIVITY 0.02f

typedef struct {
	uint16_t touchX[1];
	uint16_t touchY[1];
	uint32_t lastTouch;
	uint8_t touchDetected;
	uint8_t touchUpdate;
} GUITouchState;

// forward declaration for function pointers below
typedef struct GUIElement GUIElement;

typedef void (*GUIElementHandler)(GUIElement *button,
		GUITouchState *touchState);
typedef void (*GUIElementRenderer)(GUIElement *button);
typedef void (*GUICallback)(GUIElement *button);

// element state flags
enum {
	GUI_OFF = 1,
	GUI_ON = 2,
	GUI_HOVER = 4,
	GUI_DIRTY = 8,
	GUI_ONOFF_MASK = GUI_ON | GUI_OFF
};

typedef struct {
	const uint8_t *pixels;
	uint16_t spriteWidth;
	uint16_t spriteHeight;
	uint8_t numSprites;
	uint32_t format;
} SpriteSheet;

struct GUIElement {
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	uint32_t state;
	char* label;
	void* userData;
	SpriteSheet *sprite;
	GUIElementHandler handler;
	GUIElementRenderer render;
	GUICallback callback;
	uint8_t id;
};

typedef struct {
	float value;
	float startValue;
	float sensitivity;
	uint16_t startX;
	uint16_t startY;
} DialButtonState;

typedef struct {
	float value;
} PushButtonState;

typedef struct {
	GUIElement **items;
	uint32_t bgColor;
	uint32_t textColor;
	sFONT *font;
	uint8_t numItems;
	int8_t selected;
} GUI;

void handlePushButton(GUIElement *bt, GUITouchState *touch);
void renderPushButton(GUIElement *bt);

void handleDialButton(GUIElement *bt, GUITouchState *touch);
void renderDialButton(GUIElement *bt);

void drawSprite(uint16_t x, uint16_t y, uint8_t id, SpriteSheet *sprite);
void drawBitmapRaw(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
		uint8_t *pixels, uint32_t colorMode);

GUI *initGUI(uint8_t num, sFONT *font, uint32_t bgCol, uint32_t textCol);
void guiForceRedraw(GUI *gui);
void guiUpdate(GUI *gui, GUITouchState *touch);
void guiUpdateTouch(TS_StateTypeDef *raw, GUITouchState *touch);

GUIElement *guiElementCommon(uint8_t id, char *label, uint16_t x, uint16_t y,
		SpriteSheet *sprite, GUICallback cb);
GUIElement *guiPushButton(uint8_t id, char *label, uint16_t x, uint16_t y,
		float val, SpriteSheet *sprite, GUICallback cb);
GUIElement *guiDialButton(uint8_t id, char *label, uint16_t x, uint16_t y,
		float val, float sens, SpriteSheet *sprite, GUICallback cb);
