/////////////////////////////////////////////////////////////////////////

//

//  Project:  ItsyBits Window Style

//  Module:  itsybits.h

//

//  Include file for the ItsyBits support module (itsybits.c)

//

/////////////////////////////////////////////////////////////////////////



#ifndef _ITSYBITS_H_

#define _ITSYBITS_H_



#define IBS_HORZCAPTION    0x4000L

#define IBS_VERTCAPTION    0x8000L



BOOL    WINAPI ibInit( HANDLE hInstance ) ;     //obsolete!

UINT    WINAPI ibGetCaptionSize( HWND hWnd  ) ;

UINT    WINAPI ibSetCaptionSize( HWND hWnd, UINT nSize ) ;

LRESULT WINAPI ibDefWindowProc( HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam ) ;

VOID    WINAPI ibAdjustWindowRect( LPRECT lprc, DWORD dwStyle, BOOL fMenu ) ;



#endif



#ifndef _RGB_H_

#define _RGB_H_

         

   // Some mildly useful macros for the standard 16 colors

   #define RGBBLACK     RGB(0,0,0)

   #define RGBRED       RGB(128,0,0)

   #define RGBGREEN     RGB(0,128,0)

   #define RGBBLUE      RGB(0,0,128)

   

   #define RGBBROWN     RGB(128,128,0)

   #define RGBMAGENTA   RGB(128,0,128)

   #define RGBCYAN      RGB(0,128,128)

   #define RGBLTGRAY    RGB(192,192,192)

   

   #define RGBGRAY      RGB(128,128,128)

   #define RGBLTRED     RGB(255,0,0)

   #define RGBLTGREEN   RGB(0,255,0)

   #define RGBLTBLUE    RGB(0,0,255)

   

   #define RGBYELLOW    RGB(255,255,0)

   #define RGBLTMAGENTA RGB(255,0,255)

   #define RGBLTCYAN    RGB(0,255,255)

   #define RGBWHITE     RGB(255,255,255)

#endif





////////////////////////////////////////////////////////////////////////

// End of File: itsybits.h

////////////////////////////////////////////////////////////////////////



