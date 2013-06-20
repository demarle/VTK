/*
 * tkIntPlatDecls.h --
 *
 *      This file contains the declarations for all platform dependent
 *      unsupported functions that are exported by the Tk library.  These
 *      interfaces are not guaranteed to remain the same between
 *      versions.  Use at your own risk.
 *
 * Copyright (c) 1998-1999 by Scriptics Corporation.
 * All rights reserved.
 *
 * RCS: @(#) Id
 */

#ifndef _TKINTPLATDECLS
#define _TKINTPLATDECLS

#ifdef BUILD_tk
#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT
#endif

/*
 * WARNING: This file is automatically generated by the tools/genStubs.tcl
 * script.  Any modifications to the function declarations below should be made
 * in the generic/tkInt.decls script.
 */

/* !BEGIN!: Do not edit below this line. */

/*
 * Exported function declarations:
 */

#if !defined(__WIN32__) && !defined(MAC_TCL) /* UNIX */
/* 0 */
EXTERN void             TkCreateXEventSource _ANSI_ARGS_((void));
/* 1 */
EXTERN void             TkFreeWindowId _ANSI_ARGS_((TkDisplay * dispPtr, 
                                Window w));
/* 2 */
EXTERN void             TkInitXId _ANSI_ARGS_((TkDisplay * dispPtr));
/* 3 */
EXTERN int              TkpCmapStressed _ANSI_ARGS_((Tk_Window tkwin, 
                                Colormap colormap));
/* 4 */
EXTERN void             TkpSync _ANSI_ARGS_((Display * display));
/* 5 */
EXTERN Window           TkUnixContainerId _ANSI_ARGS_((TkWindow * winPtr));
/* 6 */
EXTERN int              TkUnixDoOneXEvent _ANSI_ARGS_((Tcl_Time * timePtr));
/* 7 */
EXTERN void             TkUnixSetMenubar _ANSI_ARGS_((Tk_Window tkwin, 
                                Tk_Window menubar));
#endif /* UNIX */
#ifdef __WIN32__
/* 0 */
EXTERN char *           TkAlignImageData _ANSI_ARGS_((XImage * image, 
                                int alignment, int bitOrder));
/* Slot 1 is reserved */
/* 2 */
EXTERN void             TkGenerateActivateEvents _ANSI_ARGS_((
                                TkWindow * winPtr, int active));
/* 3 */
EXTERN unsigned long    TkpGetMS _ANSI_ARGS_((void));
/* 4 */
EXTERN void             TkPointerDeadWindow _ANSI_ARGS_((TkWindow * winPtr));
/* 5 */
EXTERN void             TkpPrintWindowId _ANSI_ARGS_((char * buf, 
                                Window window));
/* 6 */
EXTERN int              TkpScanWindowId _ANSI_ARGS_((Tcl_Interp * interp, 
                                char * string, int * idPtr));
/* 7 */
EXTERN void             TkpSetCapture _ANSI_ARGS_((TkWindow * winPtr));
/* 8 */
EXTERN void             TkpSetCursor _ANSI_ARGS_((TkpCursor cursor));
/* 9 */
EXTERN void             TkpWmSetState _ANSI_ARGS_((TkWindow * winPtr, 
                                int state));
/* 10 */
EXTERN void             TkSetPixmapColormap _ANSI_ARGS_((Pixmap pixmap, 
                                Colormap colormap));
/* 11 */
EXTERN void             TkWinCancelMouseTimer _ANSI_ARGS_((void));
/* 12 */
EXTERN void             TkWinClipboardRender _ANSI_ARGS_((
                                TkDisplay * dispPtr, UINT format));
/* 13 */
EXTERN LRESULT          TkWinEmbeddedEventProc _ANSI_ARGS_((HWND hwnd, 
                                UINT message, WPARAM wParam, LPARAM lParam));
/* 14 */
EXTERN void             TkWinFillRect _ANSI_ARGS_((HDC dc, int x, int y, 
                                int width, int height, int pixel));
/* 15 */
EXTERN COLORREF         TkWinGetBorderPixels _ANSI_ARGS_((Tk_Window tkwin, 
                                Tk_3DBorder border, int which));
/* 16 */
EXTERN HDC              TkWinGetDrawableDC _ANSI_ARGS_((Display * display, 
                                Drawable d, TkWinDCState* state));
/* 17 */
EXTERN int              TkWinGetModifierState _ANSI_ARGS_((void));
/* 18 */
EXTERN HPALETTE         TkWinGetSystemPalette _ANSI_ARGS_((void));
/* 19 */
EXTERN HWND             TkWinGetWrapperWindow _ANSI_ARGS_((Tk_Window tkwin));
/* 20 */
EXTERN int              TkWinHandleMenuEvent _ANSI_ARGS_((HWND * phwnd, 
                                UINT * pMessage, WPARAM * pwParam, 
                                LPARAM * plParam, LRESULT * plResult));
/* 21 */
EXTERN int              TkWinIndexOfColor _ANSI_ARGS_((XColor * colorPtr));
/* 22 */
EXTERN void             TkWinReleaseDrawableDC _ANSI_ARGS_((Drawable d, 
                                HDC hdc, TkWinDCState* state));
/* 23 */
EXTERN LRESULT          TkWinResendEvent _ANSI_ARGS_((WNDPROC wndproc, 
                                HWND hwnd, XEvent * eventPtr));
/* 24 */
EXTERN HPALETTE         TkWinSelectPalette _ANSI_ARGS_((HDC dc, 
                                Colormap colormap));
/* 25 */
EXTERN void             TkWinSetMenu _ANSI_ARGS_((Tk_Window tkwin, 
                                HMENU hMenu));
/* 26 */
EXTERN void             TkWinSetWindowPos _ANSI_ARGS_((HWND hwnd, 
                                HWND siblingHwnd, int pos));
/* 27 */
EXTERN void             TkWinWmCleanup _ANSI_ARGS_((HINSTANCE hInstance));
/* 28 */
EXTERN void             TkWinXCleanup _ANSI_ARGS_((HINSTANCE hInstance));
/* 29 */
EXTERN void             TkWinXInit _ANSI_ARGS_((HINSTANCE hInstance));
/* 30 */
EXTERN void             TkWinSetForegroundWindow _ANSI_ARGS_((
                                TkWindow * winPtr));
/* 31 */
EXTERN void             TkWinDialogDebug _ANSI_ARGS_((int debug));
/* 32 */
EXTERN Tcl_Obj *        TkWinGetMenuSystemDefault _ANSI_ARGS_((
                                Tk_Window tkwin, char * dbName, 
                                char * className));
/* 33 */
EXTERN int              TkWinGetPlatformId _ANSI_ARGS_((void));
#endif /* __WIN32__ */
#ifdef MAC_TCL
/* 0 */
EXTERN void             TkGenerateActivateEvents _ANSI_ARGS_((
                                TkWindow * winPtr, int active));
/* 1 */
EXTERN Pixmap           TkpCreateNativeBitmap _ANSI_ARGS_((Display * display, 
                                char * source));
/* 2 */
EXTERN void             TkpDefineNativeBitmaps _ANSI_ARGS_((void));
/* 3 */
EXTERN unsigned long    TkpGetMS _ANSI_ARGS_((void));
/* Slot 4 is reserved */
/* 5 */
EXTERN void             TkPointerDeadWindow _ANSI_ARGS_((TkWindow * winPtr));
/* 6 */
EXTERN void             TkpSetCapture _ANSI_ARGS_((TkWindow * winPtr));
/* 7 */
EXTERN void             TkpSetCursor _ANSI_ARGS_((TkpCursor cursor));
/* 8 */
EXTERN void             TkpWmSetState _ANSI_ARGS_((TkWindow * winPtr, 
                                int state));
/* Slot 9 is reserved */
/* 10 */
EXTERN void             TkAboutDlg _ANSI_ARGS_((void));
/* Slot 11 is reserved */
/* Slot 12 is reserved */
/* 13 */
EXTERN Window           TkGetTransientMaster _ANSI_ARGS_((TkWindow * winPtr));
/* 14 */
EXTERN int              TkGenerateButtonEvent _ANSI_ARGS_((int x, int y, 
                                Window window, unsigned int state));
/* Slot 15 is reserved */
/* 16 */
EXTERN void             TkGenWMDestroyEvent _ANSI_ARGS_((Tk_Window tkwin));
/* 17 */
EXTERN void             TkGenWMConfigureEvent _ANSI_ARGS_((Tk_Window tkwin, 
                                int x, int y, int width, int height, 
                                int flags));
/* 18 */
EXTERN unsigned int     TkMacButtonKeyState _ANSI_ARGS_((void));
/* 19 */
EXTERN void             TkMacClearMenubarActive _ANSI_ARGS_((void));
/* 20 */
EXTERN int              TkMacConvertEvent _ANSI_ARGS_((
                                EventRecord * eventPtr));
/* 21 */
EXTERN int              TkMacDispatchMenuEvent _ANSI_ARGS_((int menuID, 
                                int index));
/* 22 */
EXTERN void             TkMacInstallCursor _ANSI_ARGS_((int resizeOverride));
/* 23 */
EXTERN int              TkMacConvertTkEvent _ANSI_ARGS_((
                                EventRecord * eventPtr, Window window));
/* 24 */
EXTERN void             TkMacHandleTearoffMenu _ANSI_ARGS_((void));
/* Slot 25 is reserved */
/* 26 */
EXTERN void             TkMacInvalClipRgns _ANSI_ARGS_((TkWindow * winPtr));
/* 27 */
EXTERN void             TkMacDoHLEvent _ANSI_ARGS_((EventRecord * theEvent));
/* Slot 28 is reserved */
/* 29 */
EXTERN Time             TkMacGenerateTime _ANSI_ARGS_((void));
/* 30 */
EXTERN GWorldPtr        TkMacGetDrawablePort _ANSI_ARGS_((Drawable drawable));
/* 31 */
EXTERN TkWindow *       TkMacGetScrollbarGrowWindow _ANSI_ARGS_((
                                TkWindow * winPtr));
/* 32 */
EXTERN Window           TkMacGetXWindow _ANSI_ARGS_((WindowRef macWinPtr));
/* 33 */
EXTERN int              TkMacGrowToplevel _ANSI_ARGS_((WindowRef whichWindow, 
                                Point start));
/* 34 */
EXTERN void             TkMacHandleMenuSelect _ANSI_ARGS_((long mResult, 
                                int optionKeyPressed));
/* 35 */
EXTERN int              TkMacHaveAppearance _ANSI_ARGS_((void));
/* 36 */
EXTERN void             TkMacInitAppleEvents _ANSI_ARGS_((
                                Tcl_Interp * interp));
/* 37 */
EXTERN void             TkMacInitMenus _ANSI_ARGS_((Tcl_Interp * interp));
/* 38 */
EXTERN void             TkMacInvalidateWindow _ANSI_ARGS_((
                                MacDrawable * macWin, int flag));
/* 39 */
EXTERN int              TkMacIsCharacterMissing _ANSI_ARGS_((Tk_Font tkfont, 
                                unsigned int searchChar));
/* 40 */
EXTERN void             TkMacMakeRealWindowExist _ANSI_ARGS_((
                                TkWindow * winPtr));
/* 41 */
EXTERN BitMapPtr        TkMacMakeStippleMap _ANSI_ARGS_((Drawable d1, 
                                Drawable d2));
/* 42 */
EXTERN void             TkMacMenuClick _ANSI_ARGS_((void));
/* 43 */
EXTERN void             TkMacRegisterOffScreenWindow _ANSI_ARGS_((
                                Window window, GWorldPtr portPtr));
/* 44 */
EXTERN int              TkMacResizable _ANSI_ARGS_((TkWindow * winPtr));
/* Slot 45 is reserved */
/* 46 */
EXTERN void             TkMacSetHelpMenuItemCount _ANSI_ARGS_((void));
/* 47 */
EXTERN void             TkMacSetScrollbarGrow _ANSI_ARGS_((TkWindow * winPtr, 
                                int flag));
/* 48 */
EXTERN void             TkMacSetUpClippingRgn _ANSI_ARGS_((Drawable drawable));
/* 49 */
EXTERN void             TkMacSetUpGraphicsPort _ANSI_ARGS_((GC gc));
/* 50 */
EXTERN void             TkMacUpdateClipRgn _ANSI_ARGS_((TkWindow * winPtr));
/* 51 */
EXTERN void             TkMacUnregisterMacWindow _ANSI_ARGS_((
                                GWorldPtr portPtr));
/* 52 */
EXTERN int              TkMacUseMenuID _ANSI_ARGS_((short macID));
/* 53 */
EXTERN RgnHandle        TkMacVisableClipRgn _ANSI_ARGS_((TkWindow * winPtr));
/* 54 */
EXTERN void             TkMacWinBounds _ANSI_ARGS_((TkWindow * winPtr, 
                                Rect * geometry));
/* 55 */
EXTERN void             TkMacWindowOffset _ANSI_ARGS_((WindowRef wRef, 
                                int * xOffset, int * yOffset));
/* Slot 56 is reserved */
/* 57 */
EXTERN int              TkSetMacColor _ANSI_ARGS_((unsigned long pixel, 
                                RGBColor * macColor));
/* 58 */
EXTERN void             TkSetWMName _ANSI_ARGS_((TkWindow * winPtr, 
                                Tk_Uid titleUid));
/* 59 */
EXTERN void             TkSuspendClipboard _ANSI_ARGS_((void));
/* Slot 60 is reserved */
/* 61 */
EXTERN int              TkMacZoomToplevel _ANSI_ARGS_((WindowPtr whichWindow, 
                                Point where, short zoomPart));
/* 62 */
EXTERN Tk_Window        Tk_TopCoordsToWindow _ANSI_ARGS_((Tk_Window tkwin, 
                                int rootX, int rootY, int * newX, int * newY));
/* 63 */
EXTERN MacDrawable *    TkMacContainerId _ANSI_ARGS_((TkWindow * winPtr));
/* 64 */
EXTERN MacDrawable *    TkMacGetHostToplevel _ANSI_ARGS_((TkWindow * winPtr));
#endif /* MAC_TCL */

typedef struct TkIntPlatStubs {
    int magic;
    struct TkIntPlatStubHooks *hooks;

#if !defined(__WIN32__) && !defined(MAC_TCL) /* UNIX */
    void (*tkCreateXEventSource) _ANSI_ARGS_((void)); /* 0 */
    void (*tkFreeWindowId) _ANSI_ARGS_((TkDisplay * dispPtr, Window w)); /* 1 */
    void (*tkInitXId) _ANSI_ARGS_((TkDisplay * dispPtr)); /* 2 */
    int (*tkpCmapStressed) _ANSI_ARGS_((Tk_Window tkwin, Colormap colormap)); /* 3 */
    void (*tkpSync) _ANSI_ARGS_((Display * display)); /* 4 */
    Window (*tkUnixContainerId) _ANSI_ARGS_((TkWindow * winPtr)); /* 5 */
    int (*tkUnixDoOneXEvent) _ANSI_ARGS_((Tcl_Time * timePtr)); /* 6 */
    void (*tkUnixSetMenubar) _ANSI_ARGS_((Tk_Window tkwin, Tk_Window menubar)); /* 7 */
#endif /* UNIX */
#ifdef __WIN32__
    char * (*tkAlignImageData) _ANSI_ARGS_((XImage * image, int alignment, int bitOrder)); /* 0 */
    void *reserved1;
    void (*tkGenerateActivateEvents) _ANSI_ARGS_((TkWindow * winPtr, int active)); /* 2 */
    unsigned long (*tkpGetMS) _ANSI_ARGS_((void)); /* 3 */
    void (*tkPointerDeadWindow) _ANSI_ARGS_((TkWindow * winPtr)); /* 4 */
    void (*tkpPrintWindowId) _ANSI_ARGS_((char * buf, Window window)); /* 5 */
    int (*tkpScanWindowId) _ANSI_ARGS_((Tcl_Interp * interp, char * string, int * idPtr)); /* 6 */
    void (*tkpSetCapture) _ANSI_ARGS_((TkWindow * winPtr)); /* 7 */
    void (*tkpSetCursor) _ANSI_ARGS_((TkpCursor cursor)); /* 8 */
    void (*tkpWmSetState) _ANSI_ARGS_((TkWindow * winPtr, int state)); /* 9 */
    void (*tkSetPixmapColormap) _ANSI_ARGS_((Pixmap pixmap, Colormap colormap)); /* 10 */
    void (*tkWinCancelMouseTimer) _ANSI_ARGS_((void)); /* 11 */
    void (*tkWinClipboardRender) _ANSI_ARGS_((TkDisplay * dispPtr, UINT format)); /* 12 */
    LRESULT (*tkWinEmbeddedEventProc) _ANSI_ARGS_((HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)); /* 13 */
    void (*tkWinFillRect) _ANSI_ARGS_((HDC dc, int x, int y, int width, int height, int pixel)); /* 14 */
    COLORREF (*tkWinGetBorderPixels) _ANSI_ARGS_((Tk_Window tkwin, Tk_3DBorder border, int which)); /* 15 */
    HDC (*tkWinGetDrawableDC) _ANSI_ARGS_((Display * display, Drawable d, TkWinDCState* state)); /* 16 */
    int (*tkWinGetModifierState) _ANSI_ARGS_((void)); /* 17 */
    HPALETTE (*tkWinGetSystemPalette) _ANSI_ARGS_((void)); /* 18 */
    HWND (*tkWinGetWrapperWindow) _ANSI_ARGS_((Tk_Window tkwin)); /* 19 */
    int (*tkWinHandleMenuEvent) _ANSI_ARGS_((HWND * phwnd, UINT * pMessage, WPARAM * pwParam, LPARAM * plParam, LRESULT * plResult)); /* 20 */
    int (*tkWinIndexOfColor) _ANSI_ARGS_((XColor * colorPtr)); /* 21 */
    void (*tkWinReleaseDrawableDC) _ANSI_ARGS_((Drawable d, HDC hdc, TkWinDCState* state)); /* 22 */
    LRESULT (*tkWinResendEvent) _ANSI_ARGS_((WNDPROC wndproc, HWND hwnd, XEvent * eventPtr)); /* 23 */
    HPALETTE (*tkWinSelectPalette) _ANSI_ARGS_((HDC dc, Colormap colormap)); /* 24 */
    void (*tkWinSetMenu) _ANSI_ARGS_((Tk_Window tkwin, HMENU hMenu)); /* 25 */
    void (*tkWinSetWindowPos) _ANSI_ARGS_((HWND hwnd, HWND siblingHwnd, int pos)); /* 26 */
    void (*tkWinWmCleanup) _ANSI_ARGS_((HINSTANCE hInstance)); /* 27 */
    void (*tkWinXCleanup) _ANSI_ARGS_((HINSTANCE hInstance)); /* 28 */
    void (*tkWinXInit) _ANSI_ARGS_((HINSTANCE hInstance)); /* 29 */
    void (*tkWinSetForegroundWindow) _ANSI_ARGS_((TkWindow * winPtr)); /* 30 */
    void (*tkWinDialogDebug) _ANSI_ARGS_((int debug)); /* 31 */
    Tcl_Obj * (*tkWinGetMenuSystemDefault) _ANSI_ARGS_((Tk_Window tkwin, char * dbName, char * className)); /* 32 */
    int (*tkWinGetPlatformId) _ANSI_ARGS_((void)); /* 33 */
#endif /* __WIN32__ */
#ifdef MAC_TCL
    void (*tkGenerateActivateEvents) _ANSI_ARGS_((TkWindow * winPtr, int active)); /* 0 */
    Pixmap (*tkpCreateNativeBitmap) _ANSI_ARGS_((Display * display, char * source)); /* 1 */
    void (*tkpDefineNativeBitmaps) _ANSI_ARGS_((void)); /* 2 */
    unsigned long (*tkpGetMS) _ANSI_ARGS_((void)); /* 3 */
    void *reserved4;
    void (*tkPointerDeadWindow) _ANSI_ARGS_((TkWindow * winPtr)); /* 5 */
    void (*tkpSetCapture) _ANSI_ARGS_((TkWindow * winPtr)); /* 6 */
    void (*tkpSetCursor) _ANSI_ARGS_((TkpCursor cursor)); /* 7 */
    void (*tkpWmSetState) _ANSI_ARGS_((TkWindow * winPtr, int state)); /* 8 */
    void *reserved9;
    void (*tkAboutDlg) _ANSI_ARGS_((void)); /* 10 */
    void *reserved11;
    void *reserved12;
    Window (*tkGetTransientMaster) _ANSI_ARGS_((TkWindow * winPtr)); /* 13 */
    int (*tkGenerateButtonEvent) _ANSI_ARGS_((int x, int y, Window window, unsigned int state)); /* 14 */
    void *reserved15;
    void (*tkGenWMDestroyEvent) _ANSI_ARGS_((Tk_Window tkwin)); /* 16 */
    void (*tkGenWMConfigureEvent) _ANSI_ARGS_((Tk_Window tkwin, int x, int y, int width, int height, int flags)); /* 17 */
    unsigned int (*tkMacButtonKeyState) _ANSI_ARGS_((void)); /* 18 */
    void (*tkMacClearMenubarActive) _ANSI_ARGS_((void)); /* 19 */
    int (*tkMacConvertEvent) _ANSI_ARGS_((EventRecord * eventPtr)); /* 20 */
    int (*tkMacDispatchMenuEvent) _ANSI_ARGS_((int menuID, int index)); /* 21 */
    void (*tkMacInstallCursor) _ANSI_ARGS_((int resizeOverride)); /* 22 */
    int (*tkMacConvertTkEvent) _ANSI_ARGS_((EventRecord * eventPtr, Window window)); /* 23 */
    void (*tkMacHandleTearoffMenu) _ANSI_ARGS_((void)); /* 24 */
    void *reserved25;
    void (*tkMacInvalClipRgns) _ANSI_ARGS_((TkWindow * winPtr)); /* 26 */
    void (*tkMacDoHLEvent) _ANSI_ARGS_((EventRecord * theEvent)); /* 27 */
    void *reserved28;
    Time (*tkMacGenerateTime) _ANSI_ARGS_((void)); /* 29 */
    GWorldPtr (*tkMacGetDrawablePort) _ANSI_ARGS_((Drawable drawable)); /* 30 */
    TkWindow * (*tkMacGetScrollbarGrowWindow) _ANSI_ARGS_((TkWindow * winPtr)); /* 31 */
    Window (*tkMacGetXWindow) _ANSI_ARGS_((WindowRef macWinPtr)); /* 32 */
    int (*tkMacGrowToplevel) _ANSI_ARGS_((WindowRef whichWindow, Point start)); /* 33 */
    void (*tkMacHandleMenuSelect) _ANSI_ARGS_((long mResult, int optionKeyPressed)); /* 34 */
    int (*tkMacHaveAppearance) _ANSI_ARGS_((void)); /* 35 */
    void (*tkMacInitAppleEvents) _ANSI_ARGS_((Tcl_Interp * interp)); /* 36 */
    void (*tkMacInitMenus) _ANSI_ARGS_((Tcl_Interp * interp)); /* 37 */
    void (*tkMacInvalidateWindow) _ANSI_ARGS_((MacDrawable * macWin, int flag)); /* 38 */
    int (*tkMacIsCharacterMissing) _ANSI_ARGS_((Tk_Font tkfont, unsigned int searchChar)); /* 39 */
    void (*tkMacMakeRealWindowExist) _ANSI_ARGS_((TkWindow * winPtr)); /* 40 */
    BitMapPtr (*tkMacMakeStippleMap) _ANSI_ARGS_((Drawable d1, Drawable d2)); /* 41 */
    void (*tkMacMenuClick) _ANSI_ARGS_((void)); /* 42 */
    void (*tkMacRegisterOffScreenWindow) _ANSI_ARGS_((Window window, GWorldPtr portPtr)); /* 43 */
    int (*tkMacResizable) _ANSI_ARGS_((TkWindow * winPtr)); /* 44 */
    void *reserved45;
    void (*tkMacSetHelpMenuItemCount) _ANSI_ARGS_((void)); /* 46 */
    void (*tkMacSetScrollbarGrow) _ANSI_ARGS_((TkWindow * winPtr, int flag)); /* 47 */
    void (*tkMacSetUpClippingRgn) _ANSI_ARGS_((Drawable drawable)); /* 48 */
    void (*tkMacSetUpGraphicsPort) _ANSI_ARGS_((GC gc)); /* 49 */
    void (*tkMacUpdateClipRgn) _ANSI_ARGS_((TkWindow * winPtr)); /* 50 */
    void (*tkMacUnregisterMacWindow) _ANSI_ARGS_((GWorldPtr portPtr)); /* 51 */
    int (*tkMacUseMenuID) _ANSI_ARGS_((short macID)); /* 52 */
    RgnHandle (*tkMacVisableClipRgn) _ANSI_ARGS_((TkWindow * winPtr)); /* 53 */
    void (*tkMacWinBounds) _ANSI_ARGS_((TkWindow * winPtr, Rect * geometry)); /* 54 */
    void (*tkMacWindowOffset) _ANSI_ARGS_((WindowRef wRef, int * xOffset, int * yOffset)); /* 55 */
    void *reserved56;
    int (*tkSetMacColor) _ANSI_ARGS_((unsigned long pixel, RGBColor * macColor)); /* 57 */
    void (*tkSetWMName) _ANSI_ARGS_((TkWindow * winPtr, Tk_Uid titleUid)); /* 58 */
    void (*tkSuspendClipboard) _ANSI_ARGS_((void)); /* 59 */
    void *reserved60;
    int (*tkMacZoomToplevel) _ANSI_ARGS_((WindowPtr whichWindow, Point where, short zoomPart)); /* 61 */
    Tk_Window (*tk_TopCoordsToWindow) _ANSI_ARGS_((Tk_Window tkwin, int rootX, int rootY, int * newX, int * newY)); /* 62 */
    MacDrawable * (*tkMacContainerId) _ANSI_ARGS_((TkWindow * winPtr)); /* 63 */
    MacDrawable * (*tkMacGetHostToplevel) _ANSI_ARGS_((TkWindow * winPtr)); /* 64 */
#endif /* MAC_TCL */
} TkIntPlatStubs;

#ifdef __cplusplus
extern "C" {
#endif
extern TkIntPlatStubs *tkIntPlatStubsPtr;
#ifdef __cplusplus
}
#endif

#if defined(USE_TK_STUBS) && !defined(USE_TK_STUB_PROCS)

/*
 * Inline function declarations:
 */

#if !defined(__WIN32__) && !defined(MAC_TCL) /* UNIX */
#ifndef TkCreateXEventSource
#define TkCreateXEventSource \
        (tkIntPlatStubsPtr->tkCreateXEventSource) /* 0 */
#endif
#ifndef TkFreeWindowId
#define TkFreeWindowId \
        (tkIntPlatStubsPtr->tkFreeWindowId) /* 1 */
#endif
#ifndef TkInitXId
#define TkInitXId \
        (tkIntPlatStubsPtr->tkInitXId) /* 2 */
#endif
#ifndef TkpCmapStressed
#define TkpCmapStressed \
        (tkIntPlatStubsPtr->tkpCmapStressed) /* 3 */
#endif
#ifndef TkpSync
#define TkpSync \
        (tkIntPlatStubsPtr->tkpSync) /* 4 */
#endif
#ifndef TkUnixContainerId
#define TkUnixContainerId \
        (tkIntPlatStubsPtr->tkUnixContainerId) /* 5 */
#endif
#ifndef TkUnixDoOneXEvent
#define TkUnixDoOneXEvent \
        (tkIntPlatStubsPtr->tkUnixDoOneXEvent) /* 6 */
#endif
#ifndef TkUnixSetMenubar
#define TkUnixSetMenubar \
        (tkIntPlatStubsPtr->tkUnixSetMenubar) /* 7 */
#endif
#endif /* UNIX */
#ifdef __WIN32__
#ifndef TkAlignImageData
#define TkAlignImageData \
        (tkIntPlatStubsPtr->tkAlignImageData) /* 0 */
#endif
/* Slot 1 is reserved */
#ifndef TkGenerateActivateEvents
#define TkGenerateActivateEvents \
        (tkIntPlatStubsPtr->tkGenerateActivateEvents) /* 2 */
#endif
#ifndef TkpGetMS
#define TkpGetMS \
        (tkIntPlatStubsPtr->tkpGetMS) /* 3 */
#endif
#ifndef TkPointerDeadWindow
#define TkPointerDeadWindow \
        (tkIntPlatStubsPtr->tkPointerDeadWindow) /* 4 */
#endif
#ifndef TkpPrintWindowId
#define TkpPrintWindowId \
        (tkIntPlatStubsPtr->tkpPrintWindowId) /* 5 */
#endif
#ifndef TkpScanWindowId
#define TkpScanWindowId \
        (tkIntPlatStubsPtr->tkpScanWindowId) /* 6 */
#endif
#ifndef TkpSetCapture
#define TkpSetCapture \
        (tkIntPlatStubsPtr->tkpSetCapture) /* 7 */
#endif
#ifndef TkpSetCursor
#define TkpSetCursor \
        (tkIntPlatStubsPtr->tkpSetCursor) /* 8 */
#endif
#ifndef TkpWmSetState
#define TkpWmSetState \
        (tkIntPlatStubsPtr->tkpWmSetState) /* 9 */
#endif
#ifndef TkSetPixmapColormap
#define TkSetPixmapColormap \
        (tkIntPlatStubsPtr->tkSetPixmapColormap) /* 10 */
#endif
#ifndef TkWinCancelMouseTimer
#define TkWinCancelMouseTimer \
        (tkIntPlatStubsPtr->tkWinCancelMouseTimer) /* 11 */
#endif
#ifndef TkWinClipboardRender
#define TkWinClipboardRender \
        (tkIntPlatStubsPtr->tkWinClipboardRender) /* 12 */
#endif
#ifndef TkWinEmbeddedEventProc
#define TkWinEmbeddedEventProc \
        (tkIntPlatStubsPtr->tkWinEmbeddedEventProc) /* 13 */
#endif
#ifndef TkWinFillRect
#define TkWinFillRect \
        (tkIntPlatStubsPtr->tkWinFillRect) /* 14 */
#endif
#ifndef TkWinGetBorderPixels
#define TkWinGetBorderPixels \
        (tkIntPlatStubsPtr->tkWinGetBorderPixels) /* 15 */
#endif
#ifndef TkWinGetDrawableDC
#define TkWinGetDrawableDC \
        (tkIntPlatStubsPtr->tkWinGetDrawableDC) /* 16 */
#endif
#ifndef TkWinGetModifierState
#define TkWinGetModifierState \
        (tkIntPlatStubsPtr->tkWinGetModifierState) /* 17 */
#endif
#ifndef TkWinGetSystemPalette
#define TkWinGetSystemPalette \
        (tkIntPlatStubsPtr->tkWinGetSystemPalette) /* 18 */
#endif
#ifndef TkWinGetWrapperWindow
#define TkWinGetWrapperWindow \
        (tkIntPlatStubsPtr->tkWinGetWrapperWindow) /* 19 */
#endif
#ifndef TkWinHandleMenuEvent
#define TkWinHandleMenuEvent \
        (tkIntPlatStubsPtr->tkWinHandleMenuEvent) /* 20 */
#endif
#ifndef TkWinIndexOfColor
#define TkWinIndexOfColor \
        (tkIntPlatStubsPtr->tkWinIndexOfColor) /* 21 */
#endif
#ifndef TkWinReleaseDrawableDC
#define TkWinReleaseDrawableDC \
        (tkIntPlatStubsPtr->tkWinReleaseDrawableDC) /* 22 */
#endif
#ifndef TkWinResendEvent
#define TkWinResendEvent \
        (tkIntPlatStubsPtr->tkWinResendEvent) /* 23 */
#endif
#ifndef TkWinSelectPalette
#define TkWinSelectPalette \
        (tkIntPlatStubsPtr->tkWinSelectPalette) /* 24 */
#endif
#ifndef TkWinSetMenu
#define TkWinSetMenu \
        (tkIntPlatStubsPtr->tkWinSetMenu) /* 25 */
#endif
#ifndef TkWinSetWindowPos
#define TkWinSetWindowPos \
        (tkIntPlatStubsPtr->tkWinSetWindowPos) /* 26 */
#endif
#ifndef TkWinWmCleanup
#define TkWinWmCleanup \
        (tkIntPlatStubsPtr->tkWinWmCleanup) /* 27 */
#endif
#ifndef TkWinXCleanup
#define TkWinXCleanup \
        (tkIntPlatStubsPtr->tkWinXCleanup) /* 28 */
#endif
#ifndef TkWinXInit
#define TkWinXInit \
        (tkIntPlatStubsPtr->tkWinXInit) /* 29 */
#endif
#ifndef TkWinSetForegroundWindow
#define TkWinSetForegroundWindow \
        (tkIntPlatStubsPtr->tkWinSetForegroundWindow) /* 30 */
#endif
#ifndef TkWinDialogDebug
#define TkWinDialogDebug \
        (tkIntPlatStubsPtr->tkWinDialogDebug) /* 31 */
#endif
#ifndef TkWinGetMenuSystemDefault
#define TkWinGetMenuSystemDefault \
        (tkIntPlatStubsPtr->tkWinGetMenuSystemDefault) /* 32 */
#endif
#ifndef TkWinGetPlatformId
#define TkWinGetPlatformId \
        (tkIntPlatStubsPtr->tkWinGetPlatformId) /* 33 */
#endif
#endif /* __WIN32__ */
#ifdef MAC_TCL
#ifndef TkGenerateActivateEvents
#define TkGenerateActivateEvents \
        (tkIntPlatStubsPtr->tkGenerateActivateEvents) /* 0 */
#endif
#ifndef TkpCreateNativeBitmap
#define TkpCreateNativeBitmap \
        (tkIntPlatStubsPtr->tkpCreateNativeBitmap) /* 1 */
#endif
#ifndef TkpDefineNativeBitmaps
#define TkpDefineNativeBitmaps \
        (tkIntPlatStubsPtr->tkpDefineNativeBitmaps) /* 2 */
#endif
#ifndef TkpGetMS
#define TkpGetMS \
        (tkIntPlatStubsPtr->tkpGetMS) /* 3 */
#endif
/* Slot 4 is reserved */
#ifndef TkPointerDeadWindow
#define TkPointerDeadWindow \
        (tkIntPlatStubsPtr->tkPointerDeadWindow) /* 5 */
#endif
#ifndef TkpSetCapture
#define TkpSetCapture \
        (tkIntPlatStubsPtr->tkpSetCapture) /* 6 */
#endif
#ifndef TkpSetCursor
#define TkpSetCursor \
        (tkIntPlatStubsPtr->tkpSetCursor) /* 7 */
#endif
#ifndef TkpWmSetState
#define TkpWmSetState \
        (tkIntPlatStubsPtr->tkpWmSetState) /* 8 */
#endif
/* Slot 9 is reserved */
#ifndef TkAboutDlg
#define TkAboutDlg \
        (tkIntPlatStubsPtr->tkAboutDlg) /* 10 */
#endif
/* Slot 11 is reserved */
/* Slot 12 is reserved */
#ifndef TkGetTransientMaster
#define TkGetTransientMaster \
        (tkIntPlatStubsPtr->tkGetTransientMaster) /* 13 */
#endif
#ifndef TkGenerateButtonEvent
#define TkGenerateButtonEvent \
        (tkIntPlatStubsPtr->tkGenerateButtonEvent) /* 14 */
#endif
/* Slot 15 is reserved */
#ifndef TkGenWMDestroyEvent
#define TkGenWMDestroyEvent \
        (tkIntPlatStubsPtr->tkGenWMDestroyEvent) /* 16 */
#endif
#ifndef TkGenWMConfigureEvent
#define TkGenWMConfigureEvent \
        (tkIntPlatStubsPtr->tkGenWMConfigureEvent) /* 17 */
#endif
#ifndef TkMacButtonKeyState
#define TkMacButtonKeyState \
        (tkIntPlatStubsPtr->tkMacButtonKeyState) /* 18 */
#endif
#ifndef TkMacClearMenubarActive
#define TkMacClearMenubarActive \
        (tkIntPlatStubsPtr->tkMacClearMenubarActive) /* 19 */
#endif
#ifndef TkMacConvertEvent
#define TkMacConvertEvent \
        (tkIntPlatStubsPtr->tkMacConvertEvent) /* 20 */
#endif
#ifndef TkMacDispatchMenuEvent
#define TkMacDispatchMenuEvent \
        (tkIntPlatStubsPtr->tkMacDispatchMenuEvent) /* 21 */
#endif
#ifndef TkMacInstallCursor
#define TkMacInstallCursor \
        (tkIntPlatStubsPtr->tkMacInstallCursor) /* 22 */
#endif
#ifndef TkMacConvertTkEvent
#define TkMacConvertTkEvent \
        (tkIntPlatStubsPtr->tkMacConvertTkEvent) /* 23 */
#endif
#ifndef TkMacHandleTearoffMenu
#define TkMacHandleTearoffMenu \
        (tkIntPlatStubsPtr->tkMacHandleTearoffMenu) /* 24 */
#endif
/* Slot 25 is reserved */
#ifndef TkMacInvalClipRgns
#define TkMacInvalClipRgns \
        (tkIntPlatStubsPtr->tkMacInvalClipRgns) /* 26 */
#endif
#ifndef TkMacDoHLEvent
#define TkMacDoHLEvent \
        (tkIntPlatStubsPtr->tkMacDoHLEvent) /* 27 */
#endif
/* Slot 28 is reserved */
#ifndef TkMacGenerateTime
#define TkMacGenerateTime \
        (tkIntPlatStubsPtr->tkMacGenerateTime) /* 29 */
#endif
#ifndef TkMacGetDrawablePort
#define TkMacGetDrawablePort \
        (tkIntPlatStubsPtr->tkMacGetDrawablePort) /* 30 */
#endif
#ifndef TkMacGetScrollbarGrowWindow
#define TkMacGetScrollbarGrowWindow \
        (tkIntPlatStubsPtr->tkMacGetScrollbarGrowWindow) /* 31 */
#endif
#ifndef TkMacGetXWindow
#define TkMacGetXWindow \
        (tkIntPlatStubsPtr->tkMacGetXWindow) /* 32 */
#endif
#ifndef TkMacGrowToplevel
#define TkMacGrowToplevel \
        (tkIntPlatStubsPtr->tkMacGrowToplevel) /* 33 */
#endif
#ifndef TkMacHandleMenuSelect
#define TkMacHandleMenuSelect \
        (tkIntPlatStubsPtr->tkMacHandleMenuSelect) /* 34 */
#endif
#ifndef TkMacHaveAppearance
#define TkMacHaveAppearance \
        (tkIntPlatStubsPtr->tkMacHaveAppearance) /* 35 */
#endif
#ifndef TkMacInitAppleEvents
#define TkMacInitAppleEvents \
        (tkIntPlatStubsPtr->tkMacInitAppleEvents) /* 36 */
#endif
#ifndef TkMacInitMenus
#define TkMacInitMenus \
        (tkIntPlatStubsPtr->tkMacInitMenus) /* 37 */
#endif
#ifndef TkMacInvalidateWindow
#define TkMacInvalidateWindow \
        (tkIntPlatStubsPtr->tkMacInvalidateWindow) /* 38 */
#endif
#ifndef TkMacIsCharacterMissing
#define TkMacIsCharacterMissing \
        (tkIntPlatStubsPtr->tkMacIsCharacterMissing) /* 39 */
#endif
#ifndef TkMacMakeRealWindowExist
#define TkMacMakeRealWindowExist \
        (tkIntPlatStubsPtr->tkMacMakeRealWindowExist) /* 40 */
#endif
#ifndef TkMacMakeStippleMap
#define TkMacMakeStippleMap \
        (tkIntPlatStubsPtr->tkMacMakeStippleMap) /* 41 */
#endif
#ifndef TkMacMenuClick
#define TkMacMenuClick \
        (tkIntPlatStubsPtr->tkMacMenuClick) /* 42 */
#endif
#ifndef TkMacRegisterOffScreenWindow
#define TkMacRegisterOffScreenWindow \
        (tkIntPlatStubsPtr->tkMacRegisterOffScreenWindow) /* 43 */
#endif
#ifndef TkMacResizable
#define TkMacResizable \
        (tkIntPlatStubsPtr->tkMacResizable) /* 44 */
#endif
/* Slot 45 is reserved */
#ifndef TkMacSetHelpMenuItemCount
#define TkMacSetHelpMenuItemCount \
        (tkIntPlatStubsPtr->tkMacSetHelpMenuItemCount) /* 46 */
#endif
#ifndef TkMacSetScrollbarGrow
#define TkMacSetScrollbarGrow \
        (tkIntPlatStubsPtr->tkMacSetScrollbarGrow) /* 47 */
#endif
#ifndef TkMacSetUpClippingRgn
#define TkMacSetUpClippingRgn \
        (tkIntPlatStubsPtr->tkMacSetUpClippingRgn) /* 48 */
#endif
#ifndef TkMacSetUpGraphicsPort
#define TkMacSetUpGraphicsPort \
        (tkIntPlatStubsPtr->tkMacSetUpGraphicsPort) /* 49 */
#endif
#ifndef TkMacUpdateClipRgn
#define TkMacUpdateClipRgn \
        (tkIntPlatStubsPtr->tkMacUpdateClipRgn) /* 50 */
#endif
#ifndef TkMacUnregisterMacWindow
#define TkMacUnregisterMacWindow \
        (tkIntPlatStubsPtr->tkMacUnregisterMacWindow) /* 51 */
#endif
#ifndef TkMacUseMenuID
#define TkMacUseMenuID \
        (tkIntPlatStubsPtr->tkMacUseMenuID) /* 52 */
#endif
#ifndef TkMacVisableClipRgn
#define TkMacVisableClipRgn \
        (tkIntPlatStubsPtr->tkMacVisableClipRgn) /* 53 */
#endif
#ifndef TkMacWinBounds
#define TkMacWinBounds \
        (tkIntPlatStubsPtr->tkMacWinBounds) /* 54 */
#endif
#ifndef TkMacWindowOffset
#define TkMacWindowOffset \
        (tkIntPlatStubsPtr->tkMacWindowOffset) /* 55 */
#endif
/* Slot 56 is reserved */
#ifndef TkSetMacColor
#define TkSetMacColor \
        (tkIntPlatStubsPtr->tkSetMacColor) /* 57 */
#endif
#ifndef TkSetWMName
#define TkSetWMName \
        (tkIntPlatStubsPtr->tkSetWMName) /* 58 */
#endif
#ifndef TkSuspendClipboard
#define TkSuspendClipboard \
        (tkIntPlatStubsPtr->tkSuspendClipboard) /* 59 */
#endif
/* Slot 60 is reserved */
#ifndef TkMacZoomToplevel
#define TkMacZoomToplevel \
        (tkIntPlatStubsPtr->tkMacZoomToplevel) /* 61 */
#endif
#ifndef Tk_TopCoordsToWindow
#define Tk_TopCoordsToWindow \
        (tkIntPlatStubsPtr->tk_TopCoordsToWindow) /* 62 */
#endif
#ifndef TkMacContainerId
#define TkMacContainerId \
        (tkIntPlatStubsPtr->tkMacContainerId) /* 63 */
#endif
#ifndef TkMacGetHostToplevel
#define TkMacGetHostToplevel \
        (tkIntPlatStubsPtr->tkMacGetHostToplevel) /* 64 */
#endif
#endif /* MAC_TCL */

#endif /* defined(USE_TK_STUBS) && !defined(USE_TK_STUB_PROCS) */

/* !END!: Do not edit above this line. */

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif /* _TKINTPLATDECLS */
