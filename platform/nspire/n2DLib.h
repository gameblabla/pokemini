#ifndef INCLUDE_GRAFX
#define INCLUDE_GRAFX

#include <os.h>
#include <stdarg.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

typedef int32_t Fixed;
typedef struct
{
	int32_t x;
	int32_t y;
	int32_t h;
	int32_t w;
} Rect;

#ifdef __cplusplus
extern "C" {
#endif

extern void initBuffering();
extern void updateScreen();
extern void deinitBuffering();
extern void clearBufferB();
extern uint16_t getPixel(const uint16_t*, uint32_t, uint32_t);
extern void setPixelUnsafe(uint32_t, uint32_t, uint16_t);
extern void setPixel(uint32_t, uint32_t, uint16_t);
extern void drawHLine(int32_t, int32_t, int32_t, uint16_t);
extern void drawVLine(int32_t, int32_t, int32_t, uint16_t);
extern void fillRect(int32_t, int32_t, int32_t, int32_t, uint16_t);
extern void drawLine(int32_t, int32_t, int32_t, int32_t, uint16_t);
extern void drawString(int32_t*, int32_t*, int32_t, const int8_t*, uint16_t, uint16_t);
extern void drawChar(int32_t*, int32_t*, int32_t, int8_t, uint16_t, uint16_t);
extern int32_t numberWidth(int32_t);
extern int32_t stringWidth(const int8_t*);
#define BUFF_BYTES_SIZE (320*240*2)
extern uint16_t *BUFF_BASE_ADDRESS;
#ifdef __cplusplus
}
#endif

#endif
