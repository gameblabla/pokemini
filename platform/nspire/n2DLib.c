#include <stdlib.h>
#include "n2DLib.h"
#include "n2DLib_font.h"

#ifdef __cplusplus
extern "C" {
#endif

/*             *
 *  Buffering  *
 *             */

uint16_t *BUFF_BASE_ADDRESS;

void initBuffering()
{
	uint8_t init_scr;
	
	BUFF_BASE_ADDRESS = (uint16_t*)malloc(BUFF_BYTES_SIZE);
	if(!BUFF_BASE_ADDRESS)
	{
		lcd_init(SCR_TYPE_INVALID);
		exit(0);
	}
	
	init_scr = lcd_init(SCR_320x240_565);
	if (init_scr == 0)
	{
		lcd_init(SCR_TYPE_INVALID);
		exit(0);
	}
}

void updateScreen()
{
	lcd_blit(BUFF_BASE_ADDRESS, SCR_320x240_565);
}

void deinitBuffering()
{
	if (BUFF_BASE_ADDRESS) free(BUFF_BASE_ADDRESS);
	lcd_init(SCR_TYPE_INVALID);
}

/*            *
 *  Graphics  *
 *            */

void clearBufferB()
{
	int32_t i;
	for(i = 0; i < 38400; i++)
		((uint32_t*)BUFF_BASE_ADDRESS)[i] = 0;
}


inline void setPixelUnsafe(uint32_t x, uint32_t y, uint16_t c)
{
	*((uint16_t*)BUFF_BASE_ADDRESS + x + y * 320) = c;
}


inline void setPixel(uint32_t x, uint32_t y, uint16_t c)
{
	if(x < 320 && y < 240)
		*((uint16_t*)BUFF_BASE_ADDRESS + x + y * 320) = c;
}


void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c)
{
	uint32_t _x = max(x, 0), _y = max(y, 0), _w = min(320 - _x, w - _x + x), _h = min(240 - _y, h - _y + y), i, j;
	if(_x < 320 && _y < 240)
	{
		for(j = _y; j < _y + _h; j++)
			for(i = _x; i < _x + _w; i++)
				setPixelUnsafe(i, j, c);
	}
}

/*            *
 *  Geometry  *
 *            */
 
void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint16_t c)
{
	int32_t dx = abs(x2-x1);
	int32_t dy = abs(y2-y1);
	int32_t sx = (x1 < x2)?1:-1;
	int32_t sy = (y1 < y2)?1:-1;
	int32_t err = dx-dy;
	int32_t e2;

	while (!(x1 == x2 && y1 == y2))
	{
		setPixel(x1,y1,c);
		e2 = 2*err;
		if (e2 > -dy)
		{		 
			err = err - dy;
			x1 = x1 + sx;
		}
		if (e2 < dx)
		{		 
			err = err + dx;
			y1 = y1 + sy;
		}
	}
}


/*        *
 *  Text  *
 *        */

int32_t isOutlinePixel(uint8_t* charfont, int32_t x, int32_t y)
{
	int32_t xis0 = !x, xis7 = x == 7, yis0 = !y, yis7 = y == 7;
	
	if(xis0)
	{
		if(yis0)
		{
			return !(*charfont & 0x80) && ((*charfont & 0x40) || (charfont[1] & 0x80) || (charfont[1] & 0x40));
		}
		else if(yis7)
		{
			return !(charfont[7] & 0x80) && ((charfont[7] & 0x40) || (charfont[6] & 0x80) || (charfont[6] & 0x40));
		}
		else
		{
			return !(charfont[y] & 0x80) && (
				(charfont[y - 1] & 0x80) || (charfont[y - 1] & 0x40) ||
				(charfont[y] & 0x40) ||
				(charfont[y + 1] & 0x80) || (charfont[y + 1] & 0x40));
		}
	}
	else if(xis7)
	{
		if(yis0)
		{
			return !(*charfont & 0x01) && ((*charfont & 0x02) || (charfont[1] & 0x01) || (charfont[1] & 0x02));
		}
		else if(yis7)
		{
			return !(charfont[7] & 0x01) && ((charfont[7] & 0x02) || (charfont[6] & 0x01) || (charfont[6] & 0x02));
		}
		else
		{
			return !(charfont[y] & 0x01) && (
				(charfont[y - 1] & 0x01) || (charfont[y - 1] & 0x02) ||
				(charfont[y] & 0x02) ||
				(charfont[y + 1] & 0x01) || (charfont[y + 1] & 0x02));
		}
	}
	else
	{
		int8_t b = 1 << (7 - x);
		if(yis0)
		{
			return !(*charfont & b) && (
				(*charfont & (b << 1)) || (*charfont & (b >> 1)) ||
				(charfont[1] & (b << 1)) || (charfont[1] & b) || (charfont[1] & (b >> 1)));
		}
		else if(yis7)
		{
			return !(charfont[7] & b) && (
				(charfont[7] & (b << 1)) || (charfont[7] & (b >> 1)) ||
				(charfont[6] & (b << 1)) || (charfont[6] & b) || (charfont[6] & (b >> 1)));
		}
		else
		{
			return !(charfont[y] & b) && (
				(charfont[y] & (b << 1)) || (charfont[y] & (b >> 1)) ||
				(charfont[y - 1] & (b << 1)) || (charfont[y - 1] & b) || (charfont[y - 1] & (b >> 1)) ||
				(charfont[y + 1] & (b << 1)) || (charfont[y + 1] & b) || (charfont[y + 1] & (b >> 1)));
		}
	}
}

void drawChar(int32_t *x, int32_t *y, int32_t margin, int8_t ch, uint16_t fc, uint16_t olc)
{
	int32_t i, j;
	uint8_t *charSprite;
	if(ch == '\n')
	{
		*x = margin;
		*y += 8;
	}
	else if(*y < 239)
	{
		charSprite = ch * 8 + n2DLib_font;
		// Draw charSprite as monochrome 8*8 image using given color
		for(i = 0; i < 8; i++)
		{
			for(j = 7; j >= 0; j--)
			{
				if((charSprite[i] >> j) & 1)
					setPixel(*x + (7 - j), *y + i, fc);
				else if(isOutlinePixel(charSprite, 7 - j, i))
					setPixel(*x + (7 - j), *y + i, olc);
			}
		}
		*x += 8;
	}
}

void drawString(int32_t *x, int32_t *y, int32_t _x, const int8_t *str, uint16_t fc, uint16_t olc)
{
	int32_t i, max = strlen(str) + 1;
	for(i = 0; i < max; i++)
		drawChar(x, y, _x, str[i], fc, olc);
}


#ifdef __cplusplus
}
#endif
