#include "n2DLib.h"

#ifdef __cplusplus
extern "C" {
#endif

/*             *
 *  Buffering  *
 *             */

unsigned short *BUFF_BASE_ADDRESS, *ALT_SCREEN_BASE_ADDRESS, *INV_BUFF, *temp;
void *SCREEN_BACKUP;
int swapped = 0;

void initBuffering()
{
	BUFF_BASE_ADDRESS = (unsigned short*)malloc(BUFF_BYTES_SIZE);
	if(!BUFF_BASE_ADDRESS) exit(0);
	
	SCREEN_BACKUP = *(void**)0xC0000010;
	
	// Handle monochrome screens-specific shit
	if(is_classic)
		*(int32_t*)(0xC000001C) = (*((int32_t*)0xC000001C) & ~0x0e) | 0x08;
	
	ALT_SCREEN_BASE_ADDRESS = (unsigned short*)malloc(BUFF_BYTES_SIZE + 8);
	if(!ALT_SCREEN_BASE_ADDRESS)
	{
		free(BUFF_BASE_ADDRESS);
		*((int32_t*)0xC000001C) = (*((int32_t*)0xC000001C) & ~0x0e) | 0x04;
		*(void**)0xC0000010 = SCREEN_BACKUP;
		exit(0);
	}
	
	INV_BUFF = (unsigned short*)malloc(BUFF_BYTES_SIZE);
	if(!INV_BUFF)
	{
		free(ALT_SCREEN_BASE_ADDRESS);
		free(BUFF_BASE_ADDRESS);
		*((int32_t*)0xC000001C) = (*((int32_t*)0xC000001C) & ~0x0e) | 0x04;
		*(void**)0xC0000010 = SCREEN_BACKUP;
		exit(0);
	}
	
	*(void**)0xC0000010 = ALT_SCREEN_BASE_ADDRESS;
}

void updateScreen()
{
	unsigned int *dest, *src, i, c;
	// I use different methods for refreshing the screen for GS and color screens because according to my tests, the fastest for one isn't the fastest for the other
	if(has_colors)
	{
		dest = (unsigned int*)ALT_SCREEN_BASE_ADDRESS;
		src = (unsigned int*)BUFF_BASE_ADDRESS;
		for(i = 0; i < 160 * 240; i++)
			*dest++ = *src++;
	}
	else
	{
		dest = (unsigned int*)INV_BUFF;
		src = (unsigned int*)BUFF_BASE_ADDRESS;
		for(i = 0; i < 160 * 240; i++)
		{
			c = *src++;
			c = ~c;
			// c holds two 16-bits colors, decompose them while keeping them that way
			*dest++ = ((c & 0x1f) + (((c >> 5) & 0x3f) >> 1) + ((c >> 11) & 0x1f)) / 3
				+ ((((c >> 16) & 0x1f) + (((c >> 21) & 0x3f) >> 1) + ((c >> 27) & 0x1f)) / 3 << 16);
			
		}
		
		temp = *(void**)0xC0000010;
		*(void**)0xC0000010 = INV_BUFF;
		INV_BUFF = temp;
		swapped = !swapped;
	}
}

void deinitBuffering()
{
	// Handle monochrome screens-specific shit again
	if(is_classic)
		*((int32_t*)0xC000001C) = (*((int32_t*)0xC000001C) & ~0x0e) | 0x04;
	*(void**)(0xC0000010) = SCREEN_BACKUP;
	if(swapped)
	{
		temp = *(void**)0xC0000010;
		*(void**)0xC0000010 = INV_BUFF;
		INV_BUFF = temp;
	}
	free(INV_BUFF);
	free(ALT_SCREEN_BASE_ADDRESS);
	free(BUFF_BASE_ADDRESS);
}

/*                 *
 * Hardware timers *
 *                 */
// Everything on HW timers is by aeTIos and Streetwalrus from http://www.omnimaga.org/

#define TIMER 0x900D0000
unsigned timer_ctl_bkp[2], timer_load_bkp[2];

void timer_init(unsigned timer)
{
	/* Timer configuration :
		- timers not freezed
		- timers decreasing
		- timers count to 0 and stop until a new value is loaded
	*/
	if (has_colors)
	{
		volatile unsigned *timer_ctl = (unsigned *) (TIMER + 0x08 + 0x20 * timer);
		volatile unsigned *timer_load = (unsigned *) (TIMER + 0x20 * timer);

		timer_ctl_bkp[timer] = *timer_ctl;
		timer_load_bkp[timer] = *timer_load;

		*timer_ctl &= ~(1 << 7);
		*timer_ctl = 0b01100011;
		*timer_ctl |= (1 << 7);
	}
	else
	{
		volatile unsigned *timer_ctl = (unsigned *) (TIMER + 0x08 + 0x0C * timer);
		volatile unsigned *timer_divider = (unsigned *) (TIMER + 0x04 + 0x0C * timer);

		timer_ctl_bkp[timer] = *timer_ctl;
		timer_load_bkp[timer] = *timer_divider;
		
		*timer_ctl = 0;
		*timer_divider = 0;
	}
}

void timer_restore(unsigned timer)
{
	if (has_colors)
	{
		volatile unsigned *timer_ctl = (unsigned *) (TIMER + 0x08 + 0x20 * timer);
		volatile unsigned *timer_load = (unsigned *) (TIMER + 0x20 * timer);

		*timer_ctl &= ~(1 << 7);
		*timer_ctl = timer_ctl_bkp[timer] & ~(1 << 7);
		*timer_load = timer_load_bkp[timer];
		*timer_ctl = timer_ctl_bkp[timer];
	}
	else
	{
		volatile unsigned *timer_ctl = (unsigned *) (TIMER + 0x08 + 0x0C * timer);
		volatile unsigned *timer_divider = (unsigned *) (TIMER + 0x04 + 0x0C * timer);
		volatile unsigned *timer_value = (unsigned *)(TIMER + 0x0C * timer);
		
		*timer_ctl = timer_ctl_bkp[timer];
		*timer_divider = timer_load_bkp[timer];
		*timer_value = 32;
	}
}

void timer_load(unsigned timer, unsigned value)
{
	if (has_colors)
	{
		volatile unsigned *timer_load = (unsigned *) (TIMER + 0x20 * timer);
		*timer_load = value;
	}
	else
	{
		volatile unsigned *timer_value = (unsigned *) (TIMER + 0x0C * timer);
		*timer_value = value;
	}
}

unsigned timer_read(unsigned timer)
{
	if (has_colors)
	{
		volatile unsigned *timer_value = (unsigned *) (TIMER + 0x04 + 0x20 * timer);
		return *timer_value;
	}
	else
	{
		volatile unsigned *timer_value = (unsigned *) (TIMER + 0x0C * timer);
		return *timer_value;
	}
}

/*         *
 *  Maths  *
 *         */

 /*
Example:
2.5 * 3.5 :
	xdec = 128
	ydec = 128
	xint = 2
	yint = 3
2.5 * 3 = 8.75 :
	rdec = 192
	rint = 8
*/
 
inline Fixed fixmul(Fixed x, Fixed y)
{
	// x = (xint << 8)+ xdec, y = (yint << 8)+ ydec
	Fixed xdec = x & 0xff, ydec = y & 0xff, xint = x >> 8, yint = y >> 8;
	// Like (x * y) >> 8 ; a bit slower but without any risk of overflow (noticeable when squaring and cubing)
	return ((xint * yint) << 8) + xint * ydec + xdec * yint + ((xdec * ydec) >> 8);
}
 
Fixed fixcos(Fixed angle)
{
	static Fixed cosLUT[] = { 256, 255, 255, 255, 254, 254, 253, 252, 251, 249, 248, 246, 244, 243, 241, 238, 236, 234, 231, 228, 225, 222, 219, 216, 212, 209, 205, 201, 197, 193, 189, 185, 181, 176, 171, 167, 162, 157, 152, 147, 142, 136, 131, 126, 120, 115, 109, 103, 97, 92, 86, 80, 74, 68, 62, 56, 49, 43, 37, 31, 25, 18, 12, 6, 0, -6, -12, -18, -25, -31, -37, -43, -49, -56, -62, -68, -74, -80, -86, -92, -97, -103, -109, -115, -120, -126, -131, -136, -142, -147, -152, -157, -162, -167, -171, -176, -181, -185, -189, -193, -197, -201, -205, -209, -212, -216, -219, -222, -225, -228, -231, -234, -236, -238, -241, -243, -244, -246, -248, -249, -251, -252, -253, -254, -254, -255, -255, -255, -256, -255, -255, -255, -254, -254, -253, -252, -251, -249, -248, -246, -244, -243, -241, -238, -236, -234, -231, -228, -225, -222, -219, -216, -212, -209, -205, -201, -197, -193, -189, -185, -181, -176, -171, -167, -162, -157, -152, -147, -142, -136, -131, -126, -120, -115, -109, -103, -97, -92, -86, -80, -74, -68, -62, -56, -49, -43, -37, -31, -25, -18, -12, -6, 0, 6, 12, 18, 25, 31, 37, 43, 49, 56, 62, 68, 74, 80, 86, 92, 97, 103, 109, 115, 120, 126, 131, 136, 142, 147, 152, 157, 162, 167, 171, 176, 181, 185, 189, 193, 197, 201, 205, 209, 212, 216, 219, 222, 225, 228, 231, 234, 236, 238, 241, 243, 244, 246, 248, 249, 251, 252, 253, 254, 254, 255, 255, 255 };
	return cosLUT[angle & 0xff];
}

inline void rotate(int x, int y, int cx, int cy, Fixed angle, Rect *out)
{
	x -= cx;
	y -= cy;
	out->x = fixtoi(fixmul(itofix(x), fixcos(angle)) + fixmul(itofix(y), fixsin(angle))) + cx;
	out->y = fixtoi(fixmul(itofix(x), -fixsin(angle)) + fixmul(itofix(y), fixcos(angle))) + cy;
}

void getBoundingBox(int x, int y, int w, int h, int cx, int cy, Fixed angle, Rect *out)
{
	Rect tl, tr, bl, br;
	rotate(x, y, cx, cy, angle, &tl);
	rotate(x + w, y, cx, cy, angle, &tr);
	rotate(x, y + h, cx, cy, angle, &bl);
	rotate(x + w, y + h, cx, cy, angle, &br);
	out->x = min(min(min(tl.x, tr.x), bl.x), br.x);
	out->y = min(min(min(tl.y, tr.y), bl.y), br.y);
	out->w = max(max(max(tl.x, tr.x), bl.x), br.x) - out->x;
	out->h = max(max(max(tl.y, tr.y), bl.y), br.y) - out->y;
}

inline int sq(int x)
{
	return x * x;
}

inline Fixed fixsq(Fixed x)
{
	return fixmul(x, x);
}

inline int cube(int x)
{
	return x * x * x;
}

inline Fixed fixcube(Fixed x)
{
	return fixmul(fixmul(x, x), x);
}

Fixed *pathX, *pathY;
int *pathT;
int curT;
int nbPts;
// Uses Lagrange's interpolation polynomial
// Returns whether or not the curve reached the last point
// Uses n2DLib's fixed-point numbers, very fast but not very accurate
int interpolatePathFixed(Fixed _x[], Fixed _y[], int _t[], int nb, Rect *out)
{
	static int init = 1;
	int factor, rx = 0, ry = 0;
	int i, j;
	if(init)
	{
		pathX = _x; pathY = _y; pathT = _t;
		curT = 0;
		init = 0;
		nbPts = nb;
	}
	// One polynomial gives X, the other gives Y
	// Both are a function of T
	for(i = 0; i < nbPts; i++)
	{
		factor = itofix(1);
		for(j = 0; j < nbPts; j++)
			if(i != j)
				factor = fixdiv(fixmul(factor, itofix(curT - pathT[j])), itofix(pathT[i] - pathT[j]));
		rx += fixmul(itofix(pathX[i]), factor);
		ry += fixmul(itofix(pathY[i]), factor);
	}
	
	out->x = fixtoi(rx);
	out->y = fixtoi(ry);
	if(curT == pathT[nbPts - 1])
	{
		init = 1;
		return 1;
	}
	curT++;
	return 0;
}

float *fpathX, *fpathY;
// Same with C floats, much slower but much more accurate
int interpolatePathFloat(float _x[], float _y[], int _t[], int nb, Rect *out)
{
	static int init = 1;
	float factor, rx = 0., ry = 0.;
	int i, j;
	if(init)
	{
		fpathX = _x; fpathY = _y; pathT = _t;
		curT = 0;
		init = 0;
		nbPts = nb;
	}
	
	for(i = 0; i < nbPts; i++)
	{
		factor = 1.;
		for(j = 0; j < nbPts; j++)
			if(i != j)
				factor = factor * (curT - pathT[j]) / (pathT[i] - pathT[j]);
		rx += fpathX[i] * factor;
		ry += fpathY[i] * factor;
	}
	
	out->x = (int)rx;
	out->y = (int)ry;
	if(curT == pathT[nbPts - 1])
	{
		init = 1;
		return 1;
	}
	curT++;
	return 0;
}

/*            *
 *  Graphics  *
 *            */

void clearBufferB()
{
	int i;
	for(i = 0; i < 160 * 240; i++)
		((unsigned int*)BUFF_BASE_ADDRESS)[i] = 0;
}

void clearBufferW()
{
	int i;
	for(i = 0; i < 160 * 240; i++)
		((unsigned int*)BUFF_BASE_ADDRESS)[i] = 0xffffffff;
}

void clearBuffer(unsigned short c)
{
	int i;
	unsigned int ci = (c << 16) | c;
	for(i = 0; i < 160 * 240; i++)
			*((unsigned int*)BUFF_BASE_ADDRESS + i) = ci;
}

inline unsigned short getPixel(const unsigned short *src, unsigned int x, unsigned int y)
{
	if(x < src[0] && y < src[1])
		return src[x + y * src[0] + 3];
	else
		return src[2];
}

inline void setPixelUnsafe(unsigned int x, unsigned int y, unsigned short c)
{
	*((unsigned short*)BUFF_BASE_ADDRESS + x + y * 320) = c;
}

inline void setPixel(unsigned int x, unsigned int y, unsigned short c)
{
	if(x < 320 && y < 240)
		*((unsigned short*)BUFF_BASE_ADDRESS + x + y * 320) = c;
}

inline void setPixelRGB(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b)
{
	if(x < 320 && y < 240)
		*((unsigned short*)BUFF_BASE_ADDRESS + x + y * 320) = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

void drawHLine(int y, int x1, int x2, unsigned short c)
{
	unsigned int _x1, _x2;
	if((x1 & x2) >> 31 || x1 + x2 >= 640 || (unsigned)y > 239)
	{
		return;
	}
	
	if(x1 < x2)
	{
		_x1 = max(x1, 0);
		_x2 = min(x2, 319);
	}
	else
	{
		_x1 = max(x2, 0);
		_x2 = min(x1, 319);
	}
	for(; _x1 <= _x2; _x1++)
		setPixelUnsafe(_x1, y, c);
}

void drawVLine(int x, int y1, int y2, unsigned short c)
{
	unsigned int _y1, _y2;
	if((y1 & y2) >> 31 || y1 + y2 >= 480 || (unsigned)x > 319)
	{
		return;
	}
	
	if(y1 < y2)
	{
		_y1 = max(y1, 0);
		_y2 = min(y2, 239);
	}
	else
	{
		_y1 = max(y2, 0);
		_y2 = min(y1, 239);
	}
	for(; _y1 <= _y2; _y1++)
		setPixelUnsafe(x, _y1, c);
}

void fillRect(int x, int y, int w, int h, unsigned short c)
{
	unsigned int _x = max(x, 0), _y = max(y, 0), _w = min(320 - _x, w - _x + x), _h = min(240 - _y, h - _y + y), i, j;
	if(_x < 320 && _y < 240)
	{
		for(j = _y; j < _y + _h; j++)
			for(i = _x; i < _x + _w; i++)
				setPixelUnsafe(i, j, c);
	}
}

void drawSprite(const unsigned short *src, int _x, int _y)
{
	int x, y, w = src[0] + _x, h = src[1] + _y, c = 3;
	for(y = _y; y < h; y++)
	{
		for(x = _x; x < w; x++, c++)
		{
			if(src[c] != src[2])
				setPixel(x, y, src[c]);
		}
		if(y > 239) break;
	}
}

void drawSpritePart(const unsigned short *src, int _x, int _y, const Rect* part)
{
	unsigned short c;
	int x, y, w = part->w + _x, h = part->h + _y, z = part->x, t = part->y;
	for(y = _y; y < h; y++, t++)
	{
		for(x = _x, z = part->x; x < w; x++, z++)
		{
			c = getPixel(src, z, t);
			if(c != src[2])
				setPixel(x, y, c);
			if(x > 319) break;
		}
		if(y > 239) break;
	}
}

void drawSpriteScaled(const unsigned short* source, const Rect* info)
{
	Fixed dx = itofix(source[0]) / info->w;
	Fixed dy = itofix(source[1]) / info->h;
	int x, y, _x = info->x + info->w / 2, _y = info->y + info->h / 2;
	Fixed tx = 0, ty = 0;
	unsigned short c;
	
	for(y = info->y - info->h / 2; y < _y; y++, ty += dy)
	{
		for(x = info->x - info->w / 2, tx = 0; x < _x; x++, tx += dx)
		{
			c = getPixel(source, fixtoi(tx), fixtoi(ty));
			if(c != source[2])
				setPixel(x, y, c);
			if(x > 319) break;
		}
		if(y > 239) break;
	}
}

void drawSpriteRotated(const unsigned short* source, const Rect* sr, const Rect* rc, Fixed angle)
{
	Rect defaultRect = { source[0] / 2, source[1] / 2, 0, 0 };
	Rect fr;
	unsigned short currentPixel;
	Fixed dX = fixcos(angle), dY = fixsin(angle);
	
	if(rc == NULL)
		rc = &defaultRect;
	
	getBoundingBox(-rc->x, -rc->y, source[0], source[1], 0, 0, angle, &fr);
	fr.x += sr->x;
	fr.y += sr->y;
	fr.w += fr.x;
	fr.h += fr.y;
	
	Rect cp, lsp, cdrp;
	
	// Feed fixed-point to get fixed-point
	rotate(itofix(fr.x - sr->x), itofix(fr.y - sr->y), 0, 0, -angle, &lsp);
	
	for(cp.y = fr.y; cp.y <= fr.h; cp.y++)
	{
		cdrp.x = lsp.x;
		cdrp.y = lsp.y;
		
		for(cp.x = fr.x; cp.x <= fr.w; cp.x++)
		{
			if(cp.x >= 0 && cp.x < 320 && cp.y >= 0 && cp.y < 240)
			{
				currentPixel = getPixel(source, fixtoi(cdrp.x) + rc->x, fixtoi(cdrp.y) + rc->y);
				if(currentPixel != source[2])
				{
					setPixelUnsafe(cp.x, cp.y, currentPixel);
				}
			}
			cdrp.x += dX;
			cdrp.y += dY;
		}
		
		lsp.x -= dY;
		lsp.y += dX;
	}
}

#ifdef __cplusplus
}
#endif
