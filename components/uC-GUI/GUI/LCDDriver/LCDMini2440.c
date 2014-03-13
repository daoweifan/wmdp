/*
 *File        : LCDMini2440.C
 *Purpose     : Driver for FriendlyARM board
 */

#include "LCD.h"
#include "LCD_Private.h"              /* include LCDConf.h */
#include "LCDSIM.h"
#include "GUI_Private.h"
#include "mini2440_lcd.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#if LCD_BITSPERPIXEL <= 8
  #define PIXELINDEX U8
#else
  #define PIXELINDEX WORD
#endif

#ifdef LCD_ASSERT
  #undef LCD_ASSERT
#endif
#define LCD_ASSERT(v) ASSERT(v)

#ifndef LCD_DISPLAY_INDEX
  #define LCD_DISPLAY_INDEX 0
#endif

#define COLORINDEX LCD_COLORINDEX
#define BKCOLORINDEX LCD_BKCOLORINDEX

/*********************************************************************
*
*       Macros for internal use
*/

#define SETPIXEL(x, y, c)	LCD_L0_SetPixelIndex(x, y, c)
#define GETPIXEL(x, y)		LCD_L0_GetPixelIndex(x,y)
#define XORPIXEL(x, y)		XorPixel(x,y)

/*********************************************************************
*
*       XorPixel
*/

static void XorPixel(int x, int y) {
	unsigned int Index = LCD_L0_GetPixelIndex(x,y);
	LCD_L0_SetPixelIndex(x, y, LCD_NUM_COLORS-1-Index);
}

/*********************************************************************
*
*       DrawBitLine1BPP
*/
static void DrawBitLine1BPP(int x, int y, U8 const*p, int Diff, int xsize, const LCD_PIXELINDEX*pTrans) {
  LCD_PIXELINDEX Index0 = *(pTrans+0);
  LCD_PIXELINDEX Index1 = *(pTrans+1);
  x+=Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS|LCD_DRAWMODE_XOR)) {
  case 0:    /* Write mode */
    do {
      LCD_L0_SetPixelIndex(x++,y, (*p & (0x80>>Diff)) ? Index1 : Index0);
			if (++Diff==8) {
        Diff=0;
				p++;
			}
		} while (--xsize);
    break;
  case LCD_DRAWMODE_TRANS:
    do {
  		if (*p & (0x80>>Diff))
        LCD_L0_SetPixelIndex(x,y, Index1);
      x++;
			if (++Diff==8) {
        Diff=0;
				p++;
			}
		} while (--xsize);
    break;
  case LCD_DRAWMODE_XOR:;
    do {
  		if (*p & (0x80>>Diff)) {
        int Pixel = LCD_L0_GetPixelIndex(x,y);
        LCD_L0_SetPixelIndex(x,y, LCD_NUM_COLORS-1-Pixel);
      }
      x++;
			if (++Diff==8) {
        Diff=0;
				p++;
			}
		} while (--xsize);
    break;
	}
}

/*********************************************************************
*
*       DrawBitLine2BPP
*/
#if (LCD_MAX_LOG_COLORS > 2)
static void DrawBitLine2BPP(int x, int y, U8 const * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixels = *p;
  int CurrentPixel = Diff;
  x += Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
    case 0:
      if (pTrans) {
        do {
          int Shift = (3 - CurrentPixel) << 1;
          int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
          LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
          SETPIXEL(x++, y, PixelIndex);
          if (++CurrentPixel == 4) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xsize);
      } else {
        do {
          int Shift = (3 - CurrentPixel) << 1;
          int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
          SETPIXEL(x++, y, Index);
          if (++CurrentPixel == 4) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xsize);
      }
      break;
    case LCD_DRAWMODE_TRANS:
      if (pTrans) {
        do {
          int Shift = (3 - CurrentPixel) << 1;
          int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
          if (Index) {
            LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
            SETPIXEL(x, y, PixelIndex);
          }
          x++;
          if (++CurrentPixel == 4) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xsize);
      } else {
        do {
          int Shift = (3 - CurrentPixel) << 1;
          int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
          if (Index) {
            SETPIXEL(x, y, Index);
          }
          x++;
          if (++CurrentPixel == 4) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xsize);
      }
      break;
  }
}
#endif

/*********************************************************************
*
*       DrawBitLine4BPP
*/
#if (LCD_MAX_LOG_COLORS > 4)
static void DrawBitLine4BPP(int x, int y, U8 const * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixels = *p;
  int CurrentPixel = Diff;
  x += Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
    case 0:
      if (pTrans) {
        do {
          int Shift = (1 - CurrentPixel) << 2;
          int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
          LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
          SETPIXEL(x++, y, PixelIndex);
          if (++CurrentPixel == 2) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xsize);
      } else {
        do {
          int Shift = (1 - CurrentPixel) << 2;
          int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
          SETPIXEL(x++, y, Index);
          if (++CurrentPixel == 2) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xsize);
      }
      break;
    case LCD_DRAWMODE_TRANS:
      if (pTrans) {
        do {
          int Shift = (1 - CurrentPixel) << 2;
          int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
          if (Index) {
            LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
            SETPIXEL(x, y, PixelIndex);
          }
          x++;
          if (++CurrentPixel == 2) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xsize);
      } else {
        do {
          int Shift = (1 - CurrentPixel) << 2;
          int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
          if (Index) {
            SETPIXEL(x, y, Index);
          }
          x++;
          if (++CurrentPixel == 2) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xsize);
      }
      break;
  }
}
#endif

/*********************************************************************
*
*       DrawBitLine8BPP
*/
#if (LCD_MAX_LOG_COLORS > 16)
static void DrawBitLine8BPP(int x, int y, U8 const*p, int xsize, const LCD_PIXELINDEX*pTrans) {
  LCD_PIXELINDEX pixel;
  if ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS)==0) {
    if (pTrans) {
      for (;xsize > 0; xsize--,x++,p++) {
        pixel = *p;
        SETPIXEL(x, y, *(pTrans+pixel));
      }
    } else {
      for (;xsize > 0; xsize--,x++,p++) {
        SETPIXEL(x, y, *p);
      }
    }
  } else {   /* Handle transparent bitmap */
    if (pTrans) {
      for (; xsize > 0; xsize--, x++, p++) {
        pixel = *p;
        if (pixel) {
          SETPIXEL(x+0, y, *(pTrans+pixel));
        }
      }
    } else {
      for (; xsize > 0; xsize--, x++, p++) {
        pixel = *p;
        if (pixel) {
          SETPIXEL(x+0, y, pixel);
        }
      }
    }
  }
}
#endif

/*********************************************************************
*
*       DrawBitLine16BPP
*/
#if (LCD_BITSPERPIXEL > 8)
static void DrawBitLine16BPP(int x, int y, U16 const*p, int xsize, const LCD_PIXELINDEX*pTrans) {
  LCD_PIXELINDEX pixel;
  if ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS)==0) {
    if (pTrans) {
      for (;xsize > 0; xsize--,x++,p++) {
        pixel = *p;
        SETPIXEL(x, y, *(pTrans+pixel));
      }
    } else {
      for (;xsize > 0; xsize--,x++,p++) {
        SETPIXEL(x, y, *p);
      }
    }
  } else {   /* Handle transparent bitmap */
    if (pTrans) {
      for (; xsize > 0; xsize--, x++, p++) {
        pixel = *p;
        if (pixel) {
          SETPIXEL(x+0, y, *(pTrans+pixel));
        }
      }
    } else {
      for (; xsize > 0; xsize--, x++, p++) {
        pixel = *p;
        if (pixel) {
          SETPIXEL(x+0, y, pixel);
        }
      }
    }
  }
}
#endif

/*********************************************************************
*
*       LCD_L0_DrawPixel
*
*  Purpose:  Writes 1 pixel into the display.
*/
void LCD_L0_DrawPixel(int x, int y) {
	if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
		XORPIXEL(x, y);
	} else {
		SETPIXEL(x, y, LCD_COLORINDEX);
	}
}

/*********************************************************************
*
*       LCD_L0_DrawHLine
*/
void LCD_L0_DrawHLine(int x0, int y,  int x1) {
	if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
		for (;x0 <= x1; x0++) {
			XORPIXEL(x0, y);
		}
	} else {
		for (;x0 <= x1; x0++) {
			SETPIXEL(x0, y, LCD_COLORINDEX);
		}
	}
}

/*********************************************************************
*
*       LCD_L0_DrawVLine
*/
void LCD_L0_DrawVLine(int x, int y0,  int y1) {
	if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
		while (y0 <= y1) {
			XORPIXEL(x, y0);
			y0++;
		}
	} else {
		while (y0 <= y1) {
			SETPIXEL(x, y0, LCD_COLORINDEX);
			y0++;
		}
	}
}

/*********************************************************************
*
*       LCD_L0_FillRect
*/
void LCD_L0_FillRect(int x0, int y0, int x1, int y1) {
	for (; y0 <= y1; y0++) {
		LCD_L0_DrawHLine(x0,y0, x1);
	}
}

/*********************************************************************
*
*       LCD_L0_DrawBitmap
*/
void LCD_L0_DrawBitmap(int x0, int y0,
                       int xsize, int ysize,
                       int BitsPerPixel, 
                       int BytesPerLine,
                       const U8* pData, int Diff,
                       const LCD_PIXELINDEX* pTrans)
{
  int i;
  /*
     Use DrawBitLineXBPP
  */
  for (i=0; i<ysize; i++) {
    switch (BitsPerPixel) {
    case 1:
      DrawBitLine1BPP(x0, i+y0, pData, Diff, xsize, pTrans);
      break;
    #if (LCD_MAX_LOG_COLORS > 2)
      case 2:
        DrawBitLine2BPP(x0, i+y0, pData, Diff, xsize, pTrans);
        break;
    #endif
    #if (LCD_MAX_LOG_COLORS > 4)
      case 4:
        DrawBitLine4BPP(x0, i+y0, pData, Diff, xsize, pTrans);
        break;
    #endif
    #if (LCD_MAX_LOG_COLORS > 16)
      case 8:
        DrawBitLine8BPP(x0, i+y0, pData, xsize, pTrans);
        break;
    #endif
    #if (LCD_BITSPERPIXEL > 8)
      case 16:
        DrawBitLine16BPP(x0, i+y0, (const U16 *)pData, xsize, pTrans);
        break;
    #endif
    }
    pData += BytesPerLine;
  }
}

/*********************************************************************
*
*       LCD_L0_DrawBitmap
*
*  Purpose:
*    Sets the original position of the virtual display.
*    Has no function at this point with the PC-driver.
*/
void LCD_L0_SetOrg(int x, int y) {
  GUI_USE_PARA(x);
  GUI_USE_PARA(y);
}

/*********************************************************************
*
*       Support for verification
*
*  Purpose:
*    The following routines are implemented, but have no functionility
*    at this point. The reason is that these functions are supposed
*    to supervise the hardware, which for obvious reasons can not be
*    done in a simulation.
*/
#if LCD_VERIFY
int  LCD_GetErrStat(void) {
  return 0;
}
void LCD_ClrErrStat(void) {
}
int  LCD_GetErrCnt (void) {
  return 0;
}
#endif  

/*********************************************************************
*
*       LCD_On
*       LCD_Off
*
*  (Not supported in Simulation)
*/
void LCD_Off          (void) {Lcd_EnvidOnOff(0);}
void LCD_On           (void) {Lcd_EnvidOnOff(1);}

/*********************************************************************
*
*       LCD_L0_SetLUTEntry
*/
void LCD_L0_SetLUTEntry(U8 Pos, LCD_COLOR color) {
  //LCDSIM_SetLUTEntry(Pos, color, LCD_DISPLAY_INDEX);
}

/*********************************************************************
*
*       LCD_L0_Init
*/
int  LCD_L0_Init(void) {
	Lcd_HWInit();
	Lcd_ClearScr((0x00<<11) | (0x00<<5) | (0x00));
	return 0;
}

int  LCD_L0_CheckInit(void) {
  return 0;
} 

/*********************************************************************
*
*       LCD_L0_ReInit
*
*  Purpose:
*    This routine is supplied for compatibility and interchangability of
*    "C"-sources with embedded versions of the driver. It has no real
*    effect in the PC-version as there is simply no need to re-initialize
*    the LCD since it is just simulated anyhow.
*/
void LCD_L0_ReInit       (void) {Lcd_HWInit();}

unsigned LCD_L0_GetPixelIndex(int x, int y)  {
	return GetPixel(x,y);
}

/*********************************************************************
*
*       LCD_L0_XorPixel
*
*  Purpose:
*    Inverts 1 pixel of the display.
*/
void LCD_L0_XorPixel(int x, int y) {
	XORPIXEL(x, y);
}

/*********************************************************************
*
*       LCD_L0_SetPixelIndex
*
*  Purpose:
*    Writes 1 pixel into the display.
*/
void LCD_L0_SetPixelIndex(int x, int y, int ColorIndex) {
	PutPixel(x, y, ColorIndex);
}
