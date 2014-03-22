/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              µC/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : LCDDummy.C
Purpose     : Empty driver
              This driver does no perform any function, but it can be
              used for 2 purposes:
              a) Satisfy all externals so an application can be
                compiled and linked in target hardware even if the
                driver is not already available
              b) Template for a starting point for a new driver.
----------------------------------------------------------------------   
Adapting to a new system (creating a new driver):
  In this case, the first step is to fill the routines 
  LCD_L0_GetPixelIndex, LCD_L0_SetPixelIndex and LCD_L0_Init with
  functionality, which is sufficient to make the hardware work.
  A second (optional) step would be to optimize higher level routines. 
----------------------------------------------------------------------   
Version-Date---Author-Explanation                                        
----------------------------------------------------------------------   
1.00.00 020417 JE     a) Changed to have only to adapt _GetPixelIndex
                         and _SetPixelIndex
0.90.00 020214 JE     a) First release
---------------------------END-OF-HEADER------------------------------
*/

#include "LCD_Private.h"      /* private modul definitions & config */
#include "GUI_Private.h"
#include "GUIDebug.h"
#include "device.h"
#include "os.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

/*********************************************************************
*
*       Static functions
*
**********************************************************************
*/
static struct device_graphic_ops * r61580_ops;

/*********************************************************************
*
*       Draw Bitmap 1 BPP
*/
static void  _DrawBitLine1BPP(int x, int y, U8 const GUI_UNI_PTR *p, int Diff, int xsize, const LCD_PIXELINDEX*pTrans) {
  LCD_PIXELINDEX Index0 = *(pTrans+0);
  LCD_PIXELINDEX Index1 = *(pTrans+1);
  x += Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
    case 0:
      do {
        LCD_L0_SetPixelIndex(x++, y, (*p & (0x80 >> Diff)) ? Index1 : Index0);
			  if (++Diff == 8) {
          Diff = 0;
				  p++;
			  }
		  } while (--xsize);
      break;
    case LCD_DRAWMODE_TRANS:
      do {
  		  if (*p & (0x80 >> Diff))
          LCD_L0_SetPixelIndex(x, y, Index1);
        x++;
			  if (++Diff == 8) {
          Diff = 0;
				  p++;
			  }
		  } while (--xsize);
      break;
    case LCD_DRAWMODE_XOR:;
      do {
  		  if (*p & (0x80 >> Diff)) {
          int Pixel = LCD_L0_GetPixelIndex(x, y);
          LCD_L0_SetPixelIndex(x, y, LCD_NUM_COLORS - 1 - Pixel);
        }
        x++;
			  if (++Diff == 8) {
          Diff = 0;
				  p++;
			  }
		  } while (--xsize);
      break;
	}
}

/*********************************************************************
*
*       Draw Bitmap 2 BPP
*/
#if (LCD_MAX_LOG_COLORS > 2)
static void  _DrawBitLine2BPP(int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
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
          LCD_L0_SetPixelIndex(x++, y, PixelIndex);
          if (++CurrentPixel == 4) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xsize);
      } else {
        do {
          int Shift = (3 - CurrentPixel) << 1;
          int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
          LCD_L0_SetPixelIndex(x++, y, Index);
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
            LCD_L0_SetPixelIndex(x, y, PixelIndex);
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
            LCD_L0_SetPixelIndex(x, y, Index);
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
*       Draw Bitmap 4 BPP
*/
#if (LCD_MAX_LOG_COLORS > 4)
static void  _DrawBitLine4BPP(int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
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
          LCD_L0_SetPixelIndex(x++, y, PixelIndex);
          if (++CurrentPixel == 2) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xsize);
      } else {
        do {
          int Shift = (1 - CurrentPixel) << 2;
          int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
          LCD_L0_SetPixelIndex(x++, y, Index);
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
            LCD_L0_SetPixelIndex(x, y, PixelIndex);
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
            LCD_L0_SetPixelIndex(x, y, Index);
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
*       Draw Bitmap 8 BPP
*/
#if (LCD_MAX_LOG_COLORS > 16)
static void  _DrawBitLine8BPP(int x, int y, U8 const GUI_UNI_PTR * p, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixel;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
    case 0:
      if (pTrans) {
        for (; xsize > 0; xsize--, x++, p++) {
          Pixel = *p;
          LCD_L0_SetPixelIndex(x, y, *(pTrans + Pixel));
        }
      } else {
        for (; xsize > 0; xsize--, x++, p++) {
          LCD_L0_SetPixelIndex(x, y, *p);
        }
      }
      break;
    case LCD_DRAWMODE_TRANS:
      if (pTrans) {
        for (; xsize > 0; xsize--, x++, p++) {
          Pixel = *p;
          if (Pixel) {
            LCD_L0_SetPixelIndex(x, y, *(pTrans + Pixel));
          }
        }
      } else {
        for (; xsize > 0; xsize--, x++, p++) {
          Pixel = *p;
          if (Pixel) {
            LCD_L0_SetPixelIndex(x, y, Pixel);
          }
        }
      }
      break;
  }
}
#endif

/*********************************************************************
*
*       Draw Bitmap 16 BPP
*/
#if 0
#if (LCD_BITSPERPIXEL > 8)
static void  DrawBitLine16BPP(int x, int y, U16 const GUI_UNI_PTR * p, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX pixel;
  if ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS) == 0) {
    if (pTrans) {
      for (; xsize > 0; xsize--, x++, p++) {
        pixel = *p;
        LCD_L0_SetPixelIndex(x, y, *(pTrans + pixel));
      }
    } else {
      for (;xsize > 0; xsize--, x++, p++) {
        LCD_L0_SetPixelIndex(x, y, *p);
      }
    }
  } else {
    if (pTrans) {
      for (; xsize > 0; xsize--, x++, p++) {
        pixel = *p;
        if (pixel) {
          LCD_L0_SetPixelIndex(x, y, *(pTrans + pixel));
        }
      }
    } else {
      for (; xsize > 0; xsize--, x++, p++) {
        pixel = *p;
        if (pixel) {
          LCD_L0_SetPixelIndex(x, y, pixel);
        }
      }
    }
  }
}
#endif
#endif
#if 1
/*********************************************************************
*
*       Draw Bitmap 16 BPP
*/
#if (LCD_BITSPERPIXEL > 8)
static void  DrawBitLine16BPP(int x, int y, U16 const GUI_UNI_PTR * p, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX pixel;
  void * pf;
  if ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS) == 0) {
    if (pTrans) {
      r61580_ops->set_window(x, y, x+xsize-1, y);
      for (; xsize > 0; xsize--, x++, p++) {
        pixel = *p;
        // LCD_L0_SetPixelIndex(x, y, *(pTrans + pixel));
        pf = (void *)(pTrans + pixel);
        r61580_ops->write_ram(pf);
      }
    } else {
      // for (;xsize > 0; xsize--, x++, p++) {
        // LCD_L0_SetPixelIndex(x, y, *p);
      // }
      r61580_ops->blit_line(p, x, y, xsize);
    }
  } else {
    if (pTrans) {
      for (; xsize > 0; xsize--, x++, p++) {
        pixel = *p;
        if (pixel) {
          LCD_L0_SetPixelIndex(x, y, *(pTrans + pixel));
        }
      }
    } else {
      for (; xsize > 0; xsize--, x++, p++) {
        pixel = *p;
        if (pixel) {
          LCD_L0_SetPixelIndex(x, y, pixel);
        }
      }
    }
  }
}
#endif
#endif
/*********************************************************************
*
*       LCD_L0_DrawPixel
*
*  Purpose:  Writes 1 pixel into the display.
*/
void LCD_L0_DrawPixel(int x, int y) {
	if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
		LCD_L0_XorPixel(x, y);
	} else {
		r61580_ops->set_pixel(&LCD_COLORINDEX, x, y);
	}
}

/*********************************************************************
*
*       LCD_L0_SetPixelIndex
*
* Purpose:
*   Sets the index of the given pixel. The upper layers
*   calling this routine make sure that the coordinates are in range, so
*   that no check on the parameters needs to be performed.
*/
void LCD_L0_SetPixelIndex(int x, int y, int PixelIndex)
{
  r61580_ops->set_pixel(&PixelIndex, x, y);
}

/*********************************************************************
*
*       LCD_L0_GetPixelIndex
*
* Purpose:
*   Returns the index of the given pixel. The upper layers
*   calling this routine make sure that the coordinates are in range, so
*   that no check on the parameters needs to be performed.
*/
unsigned int LCD_L0_GetPixelIndex(int x, int y)
{
  LCD_PIXELINDEX PixelIndex;
  r61580_ops->get_pixel(&PixelIndex, x, y);
  return PixelIndex;
}

/*********************************************************************
*
*       LCD_L0_XorPixel
*/
void LCD_L0_XorPixel(int x, int y)
{
  LCD_PIXELINDEX PixelIndex = LCD_L0_GetPixelIndex(x, y);
  LCD_L0_SetPixelIndex(x, y, LCD_NUM_COLORS - PixelIndex - 1);
}

/*********************************************************************
*
*       LCD_L0_DrawHLine
*/
void LCD_L0_DrawHLine  (int x0, int y,  int x1)
{
  r61580_ops->draw_hline(&LCD_COLORINDEX, x0, x1, y);
}

/*********************************************************************
*
*       LCD_L0_DrawVLine
*/
void LCD_L0_DrawVLine  (int x, int y0,  int y1)
{
  r61580_ops->draw_vline(&LCD_COLORINDEX, x, y0, y1);
}

/*********************************************************************
*
*       LCD_L0_FillRect
*/
void LCD_L0_FillRect(int x0, int y0, int x1, int y1)
{
  r61580_ops->fill_rect(&LCD_COLORINDEX, x0, y0, x1, y1);
}

/*********************************************************************
*
*       LCD_L0_DrawBitmap
*/
void LCD_L0_DrawBitmap(int x0, int y0,
                       int xsize, int ysize,
                       int BitsPerPixel, 
                       int BytesPerLine,
                       const U8 GUI_UNI_PTR * pData, int Diff,
                       const LCD_PIXELINDEX* pTrans)
{
  int i;
  /* Use _DrawBitLineXBPP */
  for (i=0; i<ysize; i++) {
    switch (BitsPerPixel) {
    case 1:
      _DrawBitLine1BPP(x0, i + y0, pData, Diff, xsize, pTrans);
      break;
    #if (LCD_MAX_LOG_COLORS > 2)
      case 2:
        _DrawBitLine2BPP(x0, i + y0, pData, Diff, xsize, pTrans);
        break;
    #endif
    #if (LCD_MAX_LOG_COLORS > 4)
      case 4:
        _DrawBitLine4BPP(x0, i + y0, pData, Diff, xsize, pTrans);
        break;
    #endif
    #if (LCD_MAX_LOG_COLORS > 16)
      case 8:
        _DrawBitLine8BPP(x0, i + y0, pData, xsize, pTrans);
        break;
    #endif
    #if (LCD_BITSPERPIXEL > 8)
      case 16:
        DrawBitLine16BPP(x0, i + y0, (const U16 *)pData, xsize, pTrans);
        break;
    #endif
    }
    pData += BytesPerLine;
  }
}

/*********************************************************************
*
*       LCD_L0_SetOrg
*/
void LCD_L0_SetOrg(int x, int y)
{
  GUI_USE_PARA(x);
  GUI_USE_PARA(y);
}

/*********************************************************************
*
*       LCD_On / LCD_Off
*/
void LCD_On (void)
{
  //todo realize
}

void LCD_Off (void)
{
  //todo realize
}

/*********************************************************************
*
*       LCD_L0_Init
*
* Purpose:
*   Initialises the LCD-controller.
*/
int  LCD_L0_Init(void)
{
  device_t lcd;
  lcd = device_find_by_name("r61580");
  r61580_ops = (struct device_graphic_ops *)lcd->user_data;
  // r61580_ops->init();
  return 0;
}

/*********************************************************************
*
*       LCD_L0_SetLUTEntry
*/
void LCD_L0_SetLUTEntry(U8 Pos, LCD_COLOR Color)
{
  GUI_USE_PARA(Pos);
  GUI_USE_PARA(Color);
}

int GUI_Init(void);

