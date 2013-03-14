/*
 * tkUnixDefault.h --
 *
 *  This file defines the defaults for all options for all of
 *  the Tk widgets.
 *
 * Copyright (c) 1991-1994 The Regents of the University of California.
 * Copyright (c) 1994-1997 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#ifndef _TKUNIXDEFAULT
#define _TKUNIXDEFAULT

/*
 * The definitions below provide symbolic names for the default colors.
 * NORMAL_BG -    Normal background color.
 * ACTIVE_BG -    Background color when widget is active.
 * SELECT_BG -    Background color for selected text.
 * TROUGH -    Background color for troughs in scales and scrollbars.
 * INDICATOR -    Color for indicator when button is selected.
 * DISABLED -    Foreground color when widget is disabled.
 */

#define BLACK    "#000000"
#define WHITE    "#ffffff"

#define NORMAL_BG  "#d9d9d9"
#define ACTIVE_BG  "#ececec"
#define SELECT_BG  "#c3c3c3"
#define TROUGH    "#b3b3b3"
#define CHECK_INDICATOR  WHITE
#define MENU_INDICATOR  BLACK
#define DISABLED  "#a3a3a3"

/*
 * Defaults for labels, buttons, checkbuttons, and radiobuttons:
 */

#define DEF_BUTTON_ANCHOR    "center"
#define DEF_BUTTON_ACTIVE_BG_COLOR  ACTIVE_BG
#define DEF_BUTTON_ACTIVE_BG_MONO  BLACK
#define DEF_BUTTON_ACTIVE_FG_COLOR  BLACK
#define DEF_CHKRAD_ACTIVE_FG_COLOR  DEF_BUTTON_ACTIVE_FG_COLOR
#define DEF_BUTTON_ACTIVE_FG_MONO  WHITE
#define DEF_BUTTON_BG_COLOR    NORMAL_BG
#define DEF_BUTTON_BG_MONO    WHITE
#define DEF_BUTTON_BITMAP    ""
#define DEF_BUTTON_BORDER_WIDTH    "1"
#define DEF_BUTTON_CURSOR    ""
#define DEF_BUTTON_COMPOUND    "none"
#define DEF_BUTTON_COMMAND    ""
#define DEF_BUTTON_DEFAULT    "disabled"
#define DEF_BUTTON_DISABLED_FG_COLOR  DISABLED
#define DEF_BUTTON_DISABLED_FG_MONO  ""
#define DEF_BUTTON_FG      BLACK
#define DEF_CHKRAD_FG      DEF_BUTTON_FG
#define DEF_BUTTON_FONT      "TkDefaultFont"
#define DEF_BUTTON_HEIGHT    "0"
#define DEF_BUTTON_HIGHLIGHT_BG_COLOR  DEF_BUTTON_BG_COLOR
#define DEF_BUTTON_HIGHLIGHT_BG_MONO  DEF_BUTTON_BG_MONO
#define DEF_BUTTON_HIGHLIGHT    BLACK
#define DEF_LABEL_HIGHLIGHT_WIDTH  "0"
#define DEF_BUTTON_HIGHLIGHT_WIDTH  "1"
#define DEF_BUTTON_IMAGE    ((char *) NULL)
#define DEF_BUTTON_INDICATOR    "1"
#define DEF_BUTTON_JUSTIFY    "center"
#define DEF_BUTTON_OFF_VALUE    "0"
#define DEF_BUTTON_ON_VALUE    "1"
#define DEF_BUTTON_TRISTATE_VALUE  ""
#define DEF_BUTTON_OVER_RELIEF    ""
#define DEF_BUTTON_PADX      "3m"
#define DEF_LABCHKRAD_PADX    "1"
#define DEF_BUTTON_PADY      "1m"
#define DEF_LABCHKRAD_PADY    "1"
#define DEF_BUTTON_RELIEF    "raised"
#define DEF_LABCHKRAD_RELIEF    "flat"
#define DEF_BUTTON_REPEAT_DELAY    "0"
#define DEF_BUTTON_REPEAT_INTERVAL  "0"
#define DEF_BUTTON_SELECT_COLOR    CHECK_INDICATOR
#define DEF_BUTTON_SELECT_MONO    BLACK
#define DEF_BUTTON_SELECT_IMAGE    ((char *) NULL)
#define DEF_BUTTON_STATE    "normal"
#define DEF_LABEL_TAKE_FOCUS    "0"
#define DEF_BUTTON_TAKE_FOCUS    ((char *) NULL)
#define DEF_BUTTON_TEXT      ""
#define DEF_BUTTON_TEXT_VARIABLE  ""
#define DEF_BUTTON_UNDERLINE    "-1"
#define DEF_BUTTON_VALUE    ""
#define DEF_BUTTON_WIDTH    "0"
#define DEF_BUTTON_WRAP_LENGTH    "0"
#define DEF_RADIOBUTTON_VARIABLE  "selectedButton"
#define DEF_CHECKBUTTON_VARIABLE  ""

/*
 * Defaults for canvases:
 */

#define DEF_CANVAS_BG_COLOR    NORMAL_BG
#define DEF_CANVAS_BG_MONO    WHITE
#define DEF_CANVAS_BORDER_WIDTH    "0"
#define DEF_CANVAS_CLOSE_ENOUGH    "1"
#define DEF_CANVAS_CONFINE    "1"
#define DEF_CANVAS_CURSOR    ""
#define DEF_CANVAS_HEIGHT    "7c"
#define DEF_CANVAS_HIGHLIGHT_BG    NORMAL_BG
#define DEF_CANVAS_HIGHLIGHT    BLACK
#define DEF_CANVAS_HIGHLIGHT_WIDTH  "1"
#define DEF_CANVAS_INSERT_BG    BLACK
#define DEF_CANVAS_INSERT_BD_COLOR  "0"
#define DEF_CANVAS_INSERT_BD_MONO  "0"
#define DEF_CANVAS_INSERT_OFF_TIME  "300"
#define DEF_CANVAS_INSERT_ON_TIME  "600"
#define DEF_CANVAS_INSERT_WIDTH    "2"
#define DEF_CANVAS_RELIEF    "flat"
#define DEF_CANVAS_SCROLL_REGION  ""
#define DEF_CANVAS_SELECT_COLOR    SELECT_BG
#define DEF_CANVAS_SELECT_MONO    BLACK
#define DEF_CANVAS_SELECT_BD_COLOR  "1"
#define DEF_CANVAS_SELECT_BD_MONO  "0"
#define DEF_CANVAS_SELECT_FG_COLOR  BLACK
#define DEF_CANVAS_SELECT_FG_MONO  WHITE
#define DEF_CANVAS_TAKE_FOCUS    ((char *) NULL)
#define DEF_CANVAS_WIDTH    "10c"
#define DEF_CANVAS_X_SCROLL_CMD    ""
#define DEF_CANVAS_X_SCROLL_INCREMENT  "0"
#define DEF_CANVAS_Y_SCROLL_CMD    ""
#define DEF_CANVAS_Y_SCROLL_INCREMENT  "0"

/*
 * Defaults for entries:
 */

#define DEF_ENTRY_BG_COLOR    WHITE
#define DEF_ENTRY_BG_MONO    WHITE
#define DEF_ENTRY_BORDER_WIDTH    "1"
#define DEF_ENTRY_CURSOR    "xterm"
#define DEF_ENTRY_DISABLED_BG_COLOR  NORMAL_BG
#define DEF_ENTRY_DISABLED_BG_MONO  WHITE
#define DEF_ENTRY_DISABLED_FG    DISABLED
#define DEF_ENTRY_EXPORT_SELECTION  "1"
#define DEF_ENTRY_FONT      "TkTextFont"
#define DEF_ENTRY_FG      BLACK
#define DEF_ENTRY_HIGHLIGHT_BG    NORMAL_BG
#define DEF_ENTRY_HIGHLIGHT    BLACK
#define DEF_ENTRY_HIGHLIGHT_WIDTH  "1"
#define DEF_ENTRY_INSERT_BG    BLACK
#define DEF_ENTRY_INSERT_BD_COLOR  "0"
#define DEF_ENTRY_INSERT_BD_MONO  "0"
#define DEF_ENTRY_INSERT_OFF_TIME  "300"
#define DEF_ENTRY_INSERT_ON_TIME  "600"
#define DEF_ENTRY_INSERT_WIDTH    "2"
#define DEF_ENTRY_JUSTIFY    "left"
#define DEF_ENTRY_READONLY_BG_COLOR  NORMAL_BG
#define DEF_ENTRY_READONLY_BG_MONO  WHITE
#define DEF_ENTRY_RELIEF    "sunken"
#define DEF_ENTRY_SCROLL_COMMAND  ""
#define DEF_ENTRY_SELECT_COLOR    SELECT_BG
#define DEF_ENTRY_SELECT_MONO    BLACK
#define DEF_ENTRY_SELECT_BD_COLOR  "0"
#define DEF_ENTRY_SELECT_BD_MONO  "0"
#define DEF_ENTRY_SELECT_FG_COLOR  BLACK
#define DEF_ENTRY_SELECT_FG_MONO  WHITE
#define DEF_ENTRY_SHOW      ((char *) NULL)
#define DEF_ENTRY_STATE      "normal"
#define DEF_ENTRY_TAKE_FOCUS    ((char *) NULL)
#define DEF_ENTRY_TEXT_VARIABLE    ""
#define DEF_ENTRY_WIDTH      "20"

/*
 * Defaults for frames:
 */

#define DEF_FRAME_BG_COLOR    NORMAL_BG
#define DEF_FRAME_BG_MONO    WHITE
#define DEF_FRAME_BORDER_WIDTH    "0"
#define DEF_FRAME_CLASS      "Frame"
#define DEF_FRAME_COLORMAP    ""
#define DEF_FRAME_CONTAINER    "0"
#define DEF_FRAME_CURSOR    ""
#define DEF_FRAME_HEIGHT    "0"
#define DEF_FRAME_HIGHLIGHT_BG    NORMAL_BG
#define DEF_FRAME_HIGHLIGHT    BLACK
#define DEF_FRAME_HIGHLIGHT_WIDTH  "0"
#define DEF_FRAME_LABEL      ""
#define DEF_FRAME_PADX      "0"
#define DEF_FRAME_PADY      "0"
#define DEF_FRAME_RELIEF    "flat"
#define DEF_FRAME_TAKE_FOCUS    "0"
#define DEF_FRAME_VISUAL    ""
#define DEF_FRAME_WIDTH      "0"

/*
 * Defaults for labelframes:
 */

#define DEF_LABELFRAME_BORDER_WIDTH  "2"
#define DEF_LABELFRAME_CLASS    "Labelframe"
#define DEF_LABELFRAME_RELIEF    "groove"
#define DEF_LABELFRAME_FG    BLACK
#define DEF_LABELFRAME_FONT    "TkDefaultFont"
#define DEF_LABELFRAME_TEXT    ""
#define DEF_LABELFRAME_LABELANCHOR  "nw"

/*
 * Defaults for listboxes:
 */

#define DEF_LISTBOX_ACTIVE_STYLE  "dotbox"
#define DEF_LISTBOX_BG_COLOR    WHITE
#define DEF_LISTBOX_BG_MONO    WHITE
#define DEF_LISTBOX_BORDER_WIDTH  "1"
#define DEF_LISTBOX_CURSOR    ""
#define DEF_LISTBOX_DISABLED_FG    DISABLED
#define DEF_LISTBOX_EXPORT_SELECTION  "1"
#define DEF_LISTBOX_FONT    "TkDefaultFont"
#define DEF_LISTBOX_FG      BLACK
#define DEF_LISTBOX_HEIGHT    "10"
#define DEF_LISTBOX_HIGHLIGHT_BG  NORMAL_BG
#define DEF_LISTBOX_HIGHLIGHT    BLACK
#define DEF_LISTBOX_HIGHLIGHT_WIDTH  "1"
#define DEF_LISTBOX_RELIEF    "sunken"
#define DEF_LISTBOX_SCROLL_COMMAND  ""
#define DEF_LISTBOX_LIST_VARIABLE  ""
#define DEF_LISTBOX_SELECT_COLOR  SELECT_BG
#define DEF_LISTBOX_SELECT_MONO    BLACK
#define DEF_LISTBOX_SELECT_BD    "0"
#define DEF_LISTBOX_SELECT_FG_COLOR  BLACK
#define DEF_LISTBOX_SELECT_FG_MONO  WHITE
#define DEF_LISTBOX_SELECT_MODE    "browse"
#define DEF_LISTBOX_SET_GRID    "0"
#define DEF_LISTBOX_STATE    "normal"
#define DEF_LISTBOX_TAKE_FOCUS    ((char *) NULL)
#define DEF_LISTBOX_WIDTH    "20"

/*
 * Defaults for individual entries of menus:
 */

#define DEF_MENU_ENTRY_ACTIVE_BG  ((char *) NULL)
#define DEF_MENU_ENTRY_ACTIVE_FG  ((char *) NULL)
#define DEF_MENU_ENTRY_ACCELERATOR  ((char *) NULL)
#define DEF_MENU_ENTRY_BG    ((char *) NULL)
#define DEF_MENU_ENTRY_BITMAP    None
#define DEF_MENU_ENTRY_COLUMN_BREAK  "0"
#define DEF_MENU_ENTRY_COMMAND    ((char *) NULL)
#define DEF_MENU_ENTRY_COMPOUND   "none"
#define DEF_MENU_ENTRY_FG    ((char *) NULL)
#define DEF_MENU_ENTRY_FONT    ((char *) NULL)
#define DEF_MENU_ENTRY_HIDE_MARGIN  "0"
#define DEF_MENU_ENTRY_IMAGE    ((char *) NULL)
#define DEF_MENU_ENTRY_INDICATOR  "1"
#define DEF_MENU_ENTRY_LABEL    ((char *) NULL)
#define DEF_MENU_ENTRY_MENU    ((char *) NULL)
#define DEF_MENU_ENTRY_OFF_VALUE  "0"
#define DEF_MENU_ENTRY_ON_VALUE    "1"
#define DEF_MENU_ENTRY_SELECT_IMAGE  ((char *) NULL)
#define DEF_MENU_ENTRY_STATE    "normal"
#define DEF_MENU_ENTRY_VALUE    ((char *) NULL)
#define DEF_MENU_ENTRY_CHECK_VARIABLE  ((char *) NULL)
#define DEF_MENU_ENTRY_RADIO_VARIABLE  "selectedButton"
#define DEF_MENU_ENTRY_SELECT    ((char *) NULL)
#define DEF_MENU_ENTRY_UNDERLINE  "-1"

/*
 * Defaults for menus overall:
 */

#define DEF_MENU_ACTIVE_BG_COLOR  ACTIVE_BG
#define DEF_MENU_ACTIVE_BG_MONO    BLACK
#define DEF_MENU_ACTIVE_BORDER_WIDTH  "1"
#define DEF_MENU_ACTIVE_FG_COLOR  BLACK
#define DEF_MENU_ACTIVE_FG_MONO    WHITE
#define DEF_MENU_BG_COLOR    NORMAL_BG
#define DEF_MENU_BG_MONO    WHITE
#define DEF_MENU_BORDER_WIDTH    "1"
#define DEF_MENU_CURSOR      "arrow"
#define DEF_MENU_DISABLED_FG_COLOR  DISABLED
#define DEF_MENU_DISABLED_FG_MONO  ""
#define DEF_MENU_FONT      "TkMenuFont"
#define DEF_MENU_FG      BLACK
#define DEF_MENU_POST_COMMAND    ""
#define DEF_MENU_RELIEF      "raised"
#define DEF_MENU_SELECT_COLOR    MENU_INDICATOR
#define DEF_MENU_SELECT_MONO    BLACK
#define DEF_MENU_TAKE_FOCUS    "0"
#define DEF_MENU_TEAROFF    "1"
#define DEF_MENU_TEAROFF_CMD    ((char *) NULL)
#define DEF_MENU_TITLE      ""
#define DEF_MENU_TYPE      "normal"

/*
 * Defaults for menubuttons:
 */

#define DEF_MENUBUTTON_ANCHOR    "center"
#define DEF_MENUBUTTON_ACTIVE_BG_COLOR  ACTIVE_BG
#define DEF_MENUBUTTON_ACTIVE_BG_MONO  BLACK
#define DEF_MENUBUTTON_ACTIVE_FG_COLOR  BLACK
#define DEF_MENUBUTTON_ACTIVE_FG_MONO  WHITE
#define DEF_MENUBUTTON_BG_COLOR    NORMAL_BG
#define DEF_MENUBUTTON_BG_MONO    WHITE
#define DEF_MENUBUTTON_BITMAP    ""
#define DEF_MENUBUTTON_BORDER_WIDTH  "1"
#define DEF_MENUBUTTON_CURSOR    ""
#define DEF_MENUBUTTON_DIRECTION  "below"
#define DEF_MENUBUTTON_DISABLED_FG_COLOR DISABLED
#define DEF_MENUBUTTON_DISABLED_FG_MONO  ""
#define DEF_MENUBUTTON_FONT    "TkDefaultFont"
#define DEF_MENUBUTTON_FG    BLACK
#define DEF_MENUBUTTON_HEIGHT    "0"
#define DEF_MENUBUTTON_HIGHLIGHT_BG_COLOR DEF_MENUBUTTON_BG_COLOR
#define DEF_MENUBUTTON_HIGHLIGHT_BG_MONO  DEF_MENUBUTTON_BG_MONO
#define DEF_MENUBUTTON_HIGHLIGHT  BLACK
#define DEF_MENUBUTTON_HIGHLIGHT_WIDTH  "0"
#define DEF_MENUBUTTON_IMAGE    ((char *) NULL)
#define DEF_MENUBUTTON_INDICATOR  "0"
#define DEF_MENUBUTTON_JUSTIFY    "center"
#define DEF_MENUBUTTON_MENU    ""
#define DEF_MENUBUTTON_PADX    "4p"
#define DEF_MENUBUTTON_PADY    "3p"
#define DEF_MENUBUTTON_RELIEF    "flat"
#define DEF_MENUBUTTON_STATE    "normal"
#define DEF_MENUBUTTON_TAKE_FOCUS  "0"
#define DEF_MENUBUTTON_TEXT    ""
#define DEF_MENUBUTTON_TEXT_VARIABLE  ""
#define DEF_MENUBUTTON_UNDERLINE  "-1"
#define DEF_MENUBUTTON_WIDTH    "0"
#define DEF_MENUBUTTON_WRAP_LENGTH  "0"

/*
 * Defaults for messages:
 */

#define DEF_MESSAGE_ANCHOR    "center"
#define DEF_MESSAGE_ASPECT    "150"
#define DEF_MESSAGE_BG_COLOR    NORMAL_BG
#define DEF_MESSAGE_BG_MONO    WHITE
#define DEF_MESSAGE_BORDER_WIDTH  "1"
#define DEF_MESSAGE_CURSOR    ""
#define DEF_MESSAGE_FG      BLACK
#define DEF_MESSAGE_FONT    "TkDefaultFont"
#define DEF_MESSAGE_HIGHLIGHT_BG  NORMAL_BG
#define DEF_MESSAGE_HIGHLIGHT    BLACK
#define DEF_MESSAGE_HIGHLIGHT_WIDTH  "0"
#define DEF_MESSAGE_JUSTIFY    "left"
#define DEF_MESSAGE_PADX    "-1"
#define DEF_MESSAGE_PADY    "-1"
#define DEF_MESSAGE_RELIEF    "flat"
#define DEF_MESSAGE_TAKE_FOCUS    "0"
#define DEF_MESSAGE_TEXT    ""
#define DEF_MESSAGE_TEXT_VARIABLE  ""
#define DEF_MESSAGE_WIDTH    "0"

/*
 * Defaults for panedwindows
 */

#define DEF_PANEDWINDOW_BG_COLOR  NORMAL_BG
#define DEF_PANEDWINDOW_BG_MONO    WHITE
#define DEF_PANEDWINDOW_BORDERWIDTH  "1"
#define DEF_PANEDWINDOW_CURSOR    ""
#define DEF_PANEDWINDOW_HANDLEPAD  "8"
#define DEF_PANEDWINDOW_HANDLESIZE  "8"
#define DEF_PANEDWINDOW_HEIGHT    ""
#define DEF_PANEDWINDOW_OPAQUERESIZE  "1"
#define DEF_PANEDWINDOW_ORIENT    "horizontal"
#define DEF_PANEDWINDOW_RELIEF    "flat"
#define DEF_PANEDWINDOW_SASHCURSOR  ""
#define DEF_PANEDWINDOW_SASHPAD    "0"
#define DEF_PANEDWINDOW_SASHRELIEF  "flat"
#define DEF_PANEDWINDOW_SASHWIDTH  "3"
#define DEF_PANEDWINDOW_SHOWHANDLE  "0"
#define DEF_PANEDWINDOW_WIDTH    ""

/*
 * Defaults for panedwindow panes
 */

#define DEF_PANEDWINDOW_PANE_AFTER  ""
#define DEF_PANEDWINDOW_PANE_BEFORE  ""
#define DEF_PANEDWINDOW_PANE_HEIGHT  ""
#define DEF_PANEDWINDOW_PANE_MINSIZE  "0"
#define DEF_PANEDWINDOW_PANE_PADX  "0"
#define DEF_PANEDWINDOW_PANE_PADY  "0"
#define DEF_PANEDWINDOW_PANE_STICKY  "nsew"
#define DEF_PANEDWINDOW_PANE_WIDTH  ""
#define DEF_PANEDWINDOW_PANE_HIDE  "0"
#define DEF_PANEDWINDOW_PANE_STRETCH  "last"

/*
 * Defaults for scales:
 */

#define DEF_SCALE_ACTIVE_BG_COLOR  ACTIVE_BG
#define DEF_SCALE_ACTIVE_BG_MONO  BLACK
#define DEF_SCALE_BG_COLOR    NORMAL_BG
#define DEF_SCALE_BG_MONO    WHITE
#define DEF_SCALE_BIG_INCREMENT    "0"
#define DEF_SCALE_BORDER_WIDTH    "1"
#define DEF_SCALE_COMMAND    ""
#define DEF_SCALE_CURSOR    ""
#define DEF_SCALE_DIGITS    "0"
#define DEF_SCALE_FONT      "TkDefaultFont"
#define DEF_SCALE_FG_COLOR    BLACK
#define DEF_SCALE_FG_MONO    BLACK
#define DEF_SCALE_FROM      "0"
#define DEF_SCALE_HIGHLIGHT_BG_COLOR  DEF_SCALE_BG_COLOR
#define DEF_SCALE_HIGHLIGHT_BG_MONO  DEF_SCALE_BG_MONO
#define DEF_SCALE_HIGHLIGHT    BLACK
#define DEF_SCALE_HIGHLIGHT_WIDTH  "1"
#define DEF_SCALE_LABEL      ""
#define DEF_SCALE_LENGTH    "100"
#define DEF_SCALE_ORIENT    "vertical"
#define DEF_SCALE_RELIEF    "flat"
#define DEF_SCALE_REPEAT_DELAY    "300"
#define DEF_SCALE_REPEAT_INTERVAL  "100"
#define DEF_SCALE_RESOLUTION    "1"
#define DEF_SCALE_TROUGH_COLOR    TROUGH
#define DEF_SCALE_TROUGH_MONO    WHITE
#define DEF_SCALE_SHOW_VALUE    "1"
#define DEF_SCALE_SLIDER_LENGTH    "30"
#define DEF_SCALE_SLIDER_RELIEF    "raised"
#define DEF_SCALE_STATE      "normal"
#define DEF_SCALE_TAKE_FOCUS    ((char *) NULL)
#define DEF_SCALE_TICK_INTERVAL    "0"
#define DEF_SCALE_TO      "100"
#define DEF_SCALE_VARIABLE    ""
#define DEF_SCALE_WIDTH      "15"

/*
 * Defaults for scrollbars:
 */

#define DEF_SCROLLBAR_ACTIVE_BG_COLOR  ACTIVE_BG
#define DEF_SCROLLBAR_ACTIVE_BG_MONO  BLACK
#define DEF_SCROLLBAR_ACTIVE_RELIEF  "raised"
#define DEF_SCROLLBAR_BG_COLOR    NORMAL_BG
#define DEF_SCROLLBAR_BG_MONO    WHITE
#define DEF_SCROLLBAR_BORDER_WIDTH  "1"
#define DEF_SCROLLBAR_COMMAND    ""
#define DEF_SCROLLBAR_CURSOR    ""
#define DEF_SCROLLBAR_EL_BORDER_WIDTH  "-1"
#define DEF_SCROLLBAR_HIGHLIGHT_BG  NORMAL_BG
#define DEF_SCROLLBAR_HIGHLIGHT    BLACK
#define DEF_SCROLLBAR_HIGHLIGHT_WIDTH  "0"
#define DEF_SCROLLBAR_JUMP    "0"
#define DEF_SCROLLBAR_ORIENT    "vertical"
#define DEF_SCROLLBAR_RELIEF    "sunken"
#define DEF_SCROLLBAR_REPEAT_DELAY  "300"
#define DEF_SCROLLBAR_REPEAT_INTERVAL  "100"
#define DEF_SCROLLBAR_TAKE_FOCUS  ((char *) NULL)
#define DEF_SCROLLBAR_TROUGH_COLOR  TROUGH
#define DEF_SCROLLBAR_TROUGH_MONO  WHITE
#define DEF_SCROLLBAR_WIDTH    "11"

/*
 * Defaults for texts:
 */

#define DEF_TEXT_AUTO_SEPARATORS  "1"
#define DEF_TEXT_BG_COLOR    WHITE
#define DEF_TEXT_BG_MONO    WHITE
#define DEF_TEXT_BLOCK_CURSOR    "0"
#define DEF_TEXT_BORDER_WIDTH    "1"
#define DEF_TEXT_CURSOR      "xterm"
#define DEF_TEXT_FG      BLACK
#define DEF_TEXT_EXPORT_SELECTION  "1"
#define DEF_TEXT_FONT      "TkFixedFont"
#define DEF_TEXT_HEIGHT      "24"
#define DEF_TEXT_HIGHLIGHT_BG    NORMAL_BG
#define DEF_TEXT_HIGHLIGHT    BLACK
#define DEF_TEXT_HIGHLIGHT_WIDTH  "1"
#define DEF_TEXT_INSERT_BG    BLACK
#define DEF_TEXT_INSERT_BD_COLOR  "0"
#define DEF_TEXT_INSERT_BD_MONO    "0"
#define DEF_TEXT_INSERT_OFF_TIME  "300"
#define DEF_TEXT_INSERT_ON_TIME    "600"
#define DEF_TEXT_INSERT_UNFOCUSSED  "none"
#define DEF_TEXT_INSERT_WIDTH    "2"
#define DEF_TEXT_MAX_UNDO    "0"
#define DEF_TEXT_PADX      "1"
#define DEF_TEXT_PADY      "1"
#define DEF_TEXT_RELIEF      "sunken"
#define DEF_TEXT_INACTIVE_SELECT_COLOR  SELECT_BG
#define DEF_TEXT_SELECT_COLOR    SELECT_BG
#define DEF_TEXT_SELECT_MONO    BLACK
#define DEF_TEXT_SELECT_BD_COLOR  "0"
#define DEF_TEXT_SELECT_BD_MONO    "0"
#define DEF_TEXT_SELECT_FG_COLOR  BLACK
#define DEF_TEXT_SELECT_FG_MONO    WHITE
#define DEF_TEXT_SELECT_RELIEF    "raised"
#define DEF_TEXT_SET_GRID    "0"
#define DEF_TEXT_SPACING1    "0"
#define DEF_TEXT_SPACING2    "0"
#define DEF_TEXT_SPACING3    "0"
#define DEF_TEXT_STATE      "normal"
#define DEF_TEXT_TABS      ""
#define DEF_TEXT_TABSTYLE    "tabular"
#define DEF_TEXT_TAKE_FOCUS    ((char *) NULL)
#define DEF_TEXT_UNDO      "0"
#define DEF_TEXT_WIDTH      "80"
#define DEF_TEXT_WRAP      "char"
#define DEF_TEXT_XSCROLL_COMMAND  ""
#define DEF_TEXT_YSCROLL_COMMAND  ""

/*
 * Defaults for canvas text:
 */

#define DEF_CANVTEXT_FONT    "TkDefaultFont"

/*
 * Defaults for toplevels (most of the defaults for frames also apply
 * to toplevels):
 */

#define DEF_TOPLEVEL_CLASS    "Toplevel"
#define DEF_TOPLEVEL_MENU    ""
#define DEF_TOPLEVEL_SCREEN    ""
#define DEF_TOPLEVEL_USE    ""

/*
 * Defaults for busy windows:
 */

#define DEF_BUSY_CURSOR      "watch"

#endif /* _TKUNIXDEFAULT */
