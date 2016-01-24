#pragma once

#define UI_BG_COLOR   ((uint32_t)0xff000000)
#define UI_TEXT_COLOR ((uint32_t)0xffffffff)
#define UI_FONT Font12

// forward declaration for function pointers below
typedef struct GUIElement GUIElement;

typedef void (*GUIElementHandler)(GUIElement *button,
		TS_StateTypeDef *touchState);
typedef void (*GUIElementRenderer)(GUIElement *button);

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
	GUIElement *items;
	uint8_t numItems;
	int8_t selected;
} GUI;

void handlePushButton(GUIElement *bt, TS_StateTypeDef *touch);
void renderPushButton(GUIElement *bt);

void handleDialButton(GUIElement *bt, TS_StateTypeDef *touch);
void renderDialButton(GUIElement *bt);

void drawSprite(uint16_t x, uint16_t y, uint8_t id, SpriteSheet *sprite);
void drawBitmapRaw(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
		uint8_t *pixels);

GUIElement *guiDialButton(uint8_t id, char *label, uint16_t x, uint16_t y, float val,
		float sens, SpriteSheet *sprite);
