#ifndef __OLED_H__
#define __OLED_H__
#include <zephyr.h>

int InitOLED();
int WriteOLED(u8_t data, u32_t flag);
int CommandOLED(u8_t cmd);
int DataOLED(u8_t data);
int ScrollVertical(u8_t n);
int ScrollHorizontal(u8_t dir, u8_t start, u8_t num, u8_t speed);
int StartScrollHorizontal();
int StopScrollHorizontal();
int SetDisplayMode(u8_t mode);
int  SetSleepMode(u8_t mode);
int SetCursor(u8_t x, u8_t y, u8_t w, u8_t h);
int DrawRect(u8_t x, u8_t y, u8_t w, u8_t h,
			u8_t r, u8_t g, u8_t b);
int DrawChar(u8_t x, u8_t y, u8_t r, u8_t g, u8_t b, char c);
int DrawString(u8_t x, u8_t y, u8_t r, u8_t g, u8_t b, const char* str);

extern const u8_t CharMap[1408];
#endif // __OLED_H__