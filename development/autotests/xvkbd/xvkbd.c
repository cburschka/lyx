/*
 * xvkbd - Virtual Keyboard for X Window System
 * (Version 3.2, 2010-03-14)
 *
 * Copyright (C) 2000-2010 by Tom Sato <VEF00200@nifty.ne.jp>
 * http://homepage3.nifty.com/tsato/xvkbd/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <fnmatch.h>
#include <limits.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xproto.h>  /* to get request code */
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Repeater.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xmu/WinUtil.h>
#include <X11/Xatom.h>

#ifdef USE_I18N
# include <X11/Xlocale.h>
#endif

#ifdef USE_XTEST
# include <X11/extensions/XTest.h>
#endif

#include "resources.h"
#define PROGRAM_NAME_WITH_VERSION "xvkbd (v3.2)"

/*
 * Default keyboard layout is hardcoded here.
 * Layout of the main keyboard can be redefined by resources.
 */
#define NUM_KEY_ROWS    25
#define NUM_KEY_COLS    25

static char *keys_normal[NUM_KEY_ROWS][NUM_KEY_COLS] = {
  { "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "BackSpace" },
  { "Escape", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "\\", "`" },
  { "Tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "Delete" },
  { "Control_L", "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "Return" },
  { "Shift_L", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "Multi_key", "Shift_R" },
  { "MainMenu", "Caps_Lock", "Alt_L", "Meta_L", "space", "Meta_R", "Alt_R",
    "Left", "Right", "Up", "Down", "Focus" },
};
static char *keys_shift[NUM_KEY_ROWS][NUM_KEY_COLS] = {
  { "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "BackSpace" },
  { "Escape", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "|", "~" },
  { "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "Delete" },
  { "Control_L", "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "Return" },
  { "Shift_L", "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "Multi_key", "Shift_R" },
  { "MainMenu", "Caps_Lock", "Alt_L", "Meta_L", "space", "Meta_R", "Alt_R",
    "Left", "Right", "Up", "Down", "Focus" },
};
static char *keys_altgr[NUM_KEY_ROWS][NUM_KEY_COLS] = { { NULL } };
static char *keys_shift_altgr[NUM_KEY_ROWS][NUM_KEY_COLS] = { { NULL } };

static char *key_labels[NUM_KEY_ROWS][NUM_KEY_COLS] = {
  { "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "Backspace" },
  { "Esc", "!\n1", "@\n2", "#\n3", "$\n4", "%\n5", "^\n6",
    "&\n7", "*\n8", "(\n9", ")\n0", "_\n-", "+\n=", "|\n\\", "~\n`" },
  { "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{\n[", "}\n]", "Del" },
  { "Control", "A", "S", "D", "F", "G", "H", "J", "K", "L", ":\n;", "\"\n'", "Return" },
  { "Shift", "Z", "X", "C", "V", "B", "N", "M", "<\n,", ">\n.", "?\n/", "Com\npose", "Shift" },
  { "MainMenu", "Caps\nLock", "Alt", "Meta", "", "Meta", "Alt",
    "left", "right", "up", "down", "Focus" },
};
static char *normal_key_labels[NUM_KEY_ROWS][NUM_KEY_COLS] = {
  { "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "Backspace" },
  { "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "\\", "`" },
  { "Tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "Del" },
  { "Ctrl", "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "Return" },
  { "Shift", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "Comp", "Shift" },
  { "MainMenu", "Caps", "Alt", "Meta", "", "Meta", "Alt",
    "left", "right", "up", "down", "Focus" },
};
static char *shift_key_labels[NUM_KEY_ROWS][NUM_KEY_COLS] = {
  { "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "Backspace" },
  { "Esc", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "|", "~" },
  { "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "Del" },
  { "Ctrl", "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "Return" },
  { "Shift", "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "Comp", "Shift" },
  { "MainMenu", "Caps", "Alt", "Meta", "", "Meta", "Alt",
    "left", "right", "up", "down", "Focus" },
};
static char *altgr_key_labels[NUM_KEY_ROWS][NUM_KEY_COLS] = { { NULL } };
static char *shift_altgr_key_labels[NUM_KEY_ROWS][NUM_KEY_COLS] = { { NULL } };


#define NUM_KEYPAD_ROWS  NUM_KEY_ROWS
#define NUM_KEYPAD_COLS  NUM_KEY_COLS

static char *keypad[NUM_KEYPAD_ROWS][NUM_KEYPAD_COLS] = {
  { "Num_Lock",  "KP_Divide",   "KP_Multiply", "Focus"       },
  { "Home",      "Up",          "Page_Up",     "KP_Add"      },
  { "Left",      "5",           "Right",       "KP_Subtract" },
  { "End",       "Down",        "Page_Down",   "KP_Enter"    },
  { "Insert",                   "Delete"                     },
};
static char *keypad_shift[NUM_KEYPAD_ROWS][NUM_KEYPAD_COLS] = {
  { "Num_Lock",  "KP_Divide",   "KP_Multiply", "Focus"       },
  { "KP_7",      "KP_8",        "KP_9",        "KP_Add"      },
  { "KP_4",      "KP_5",        "KP_6",        "KP_Subtract" },
  { "KP_1",      "KP_2",        "KP_3",        "KP_Enter"    },
  { "KP_0",                     "."                          },
};
static char *keypad_label[NUM_KEYPAD_ROWS][NUM_KEYPAD_COLS] = {
  { "Num\nLock", "/",           "*",           "Focus"       },
  { "7\nHome",   "8\nUp  ",     "9\nPgUp",     "+"           },
  { "4\nLeft",   "5\n    ",     "6\nRight",    "-"           },
  { "1\nEnd ",   "2\nDown",     "3\nPgDn",     "Enter"       },
  { "0\nIns ",                  ".\nDel "                    },
};

#define NUM_SUN_FKEY_ROWS 6
#define NUM_SUN_FKEY_COLS 3

static char *sun_fkey[NUM_SUN_FKEY_ROWS][NUM_SUN_FKEY_COLS] = {
  { "L1", "L2"  },
  { "L3", "L4"  },
  { "L5", "L6"  },
  { "L7", "L8"  },
  { "L9", "L10" },
  { "Help"      },
};
static char *sun_fkey_label[NUM_SUN_FKEY_ROWS][NUM_SUN_FKEY_COLS] = {
  { "Stop \nL1", "Again\nL2"  },
  { "Props\nL3", "Undo \nL4"  },
  { "Front\nL5", "Copy \nL6"  },
  { "Open \nL7", "Paste\nL8"  },
  { "Find \nL9", "Cut  \nL10" },
  { "Help"                    },
};

/*
 * Image for arrow keys
 */
#define up_width 7
#define up_height 13
static unsigned char up_bits[] = {
   0x08, 0x1c, 0x1c, 0x3e, 0x2a, 0x49, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
   0x08};

#define down_width 7
#define down_height 13
static unsigned char down_bits[] = {
   0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x49, 0x2a, 0x3e, 0x1c, 0x1c,
   0x08};

#define left_width 13
#define left_height 13
static unsigned char left_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x18, 0x00, 0x0e, 0x00,
   0xff, 0x1f, 0x0e, 0x00, 0x18, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00};

#define right_width 13
#define right_height 13
static unsigned char right_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x03, 0x00, 0x0e,
   0xff, 0x1f, 0x00, 0x0e, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00};

#define check_width 16
#define check_height 16
static unsigned char check_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x38, 0x00, 0x1e, 0x08, 0x0f,
  0x8c, 0x07, 0xde, 0x03, 0xfe, 0x03, 0xfc, 0x01, 0xf8, 0x00, 0xf0, 0x00,
  0x70, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00};

#define back_width 18
#define back_height 13
static unsigned char back_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00,
   0x78, 0x00, 0x00, 0xfe, 0xff, 0x03, 0xff, 0xff, 0x03, 0xfe, 0xff, 0x03,
   0x78, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00};

/*
 * Resources and options
 */
#define Offset(entry) XtOffset(struct appres_struct *, entry)
static XtResource application_resources[] = {
  { "description", "Description", XtRString, sizeof(char *),
    Offset(description), XtRImmediate,
    PROGRAM_NAME_WITH_VERSION " - virtual keyboard for X window system\n\n"
    "Copyright (C) 2000-2010 by Tom Sato <VEF00200@nifty.ne.jp>\n"
    "http://homepage3.nifty.com/tsato/xvkbd/\n\n"
    "This program is free software with ABSOLUTELY NO WARRANTY,\n"
    "distributed under the terms of the GNU General Public License.\n" },
  { "showManualCommand", "ShowManualCommand", XtRString, sizeof(char *),
    Offset(show_manual_command), XtRImmediate, "xterm -e man xvkbd &" },

  { "windowGeometry", "Geometry", XtRString, sizeof(char *),
    Offset(geometry), XtRImmediate, "" },
  { "inheritGeoemetry", "Inherit", XtRBoolean, sizeof(Boolean),
     Offset(inherit_geometry), XtRImmediate, (XtPointer)TRUE },
  { "debug", "Debug", XtRBoolean, sizeof(Boolean),
     Offset(debug), XtRImmediate, (XtPointer)FALSE },
  { "version", "Version", XtRBoolean, sizeof(Boolean),
     Offset(version), XtRImmediate, (XtPointer)FALSE },
#ifdef USE_XTEST
  { "xtest", "XTest", XtRBoolean, sizeof(Boolean),
     Offset(xtest), XtRImmediate, (XtPointer)TRUE },
#else
  { "xtest", "XTest", XtRBoolean, sizeof(Boolean),
     Offset(xtest), XtRImmediate, (XtPointer)FALSE },
#endif
  { "noSync", "NoSync", XtRBoolean, sizeof(Boolean),
     Offset(no_sync), XtRImmediate, (XtPointer)FALSE },
  { "alwaysOnTop", "AlwaysOnTop", XtRBoolean, sizeof(Boolean),
     Offset(always_on_top), XtRImmediate, (XtPointer)FALSE },
  { "wmToolbar", "WmToolbar", XtRBoolean, sizeof(Boolean),
     Offset(wm_toolbar), XtRImmediate, (XtPointer)FALSE },
  { "jumpPointer", "JumpPointer", XtRBoolean, sizeof(Boolean),
     Offset(jump_pointer), XtRImmediate, (XtPointer)TRUE },
  { "jumpPointerAlways", "JumpPointer", XtRBoolean, sizeof(Boolean),
     Offset(jump_pointer_always), XtRImmediate, (XtPointer)TRUE },
  { "jumpPointerBack", "JumpPointer", XtRBoolean, sizeof(Boolean),
     Offset(jump_pointer_back), XtRImmediate, (XtPointer)TRUE },
  { "quickModifiers", "QuickModifiers", XtRBoolean, sizeof(Boolean),
     Offset(quick_modifiers), XtRImmediate, (XtPointer)TRUE },
  { "altgrLock", "ModifiersLock", XtRBoolean, sizeof(Boolean),
     Offset(altgr_lock), XtRImmediate, (XtPointer)FALSE },
  { "shiftLock", "ModifiersLock", XtRBoolean, sizeof(Boolean),
     Offset(shift_lock), XtRImmediate, (XtPointer)FALSE },
  { "modifiersLock", "ModifiersLock", XtRBoolean, sizeof(Boolean),
     Offset(modifiers_lock), XtRImmediate, (XtPointer)FALSE },
  { "numLockState", "NumLockState", XtRBoolean, sizeof(Boolean),
     Offset(num_lock_state), XtRImmediate, (XtPointer)TRUE },
  { "autoRepeat", "AutoRepeat", XtRBoolean, sizeof(Boolean),
     Offset(auto_repeat), XtRImmediate, (XtPointer)TRUE },
  { "modalKeytop", "ModalKeytop", XtRBoolean, sizeof(Boolean),
     Offset(modal_keytop), XtRImmediate, (XtPointer)FALSE },
  { "minimizable", "Minimizable", XtRBoolean, sizeof(Boolean),
     Offset(minimizable), XtRImmediate, (XtPointer)FALSE },
  { "secure", "Secure", XtRBoolean, sizeof(Boolean),
     Offset(secure), XtRImmediate, (XtPointer)FALSE },
  { "no_root", "NoRoot", XtRBoolean, sizeof(Boolean),
     Offset(no_root), XtRImmediate, (XtPointer)FALSE },
  { "wait_idle", "Text", XtRString, sizeof(char *),
    Offset(wait_idle), XtRImmediate, "" },
  { "nonexitable", "Secure", XtRBoolean, sizeof(Boolean),
     Offset(nonexitable), XtRImmediate, (XtPointer)FALSE },
  { "modalKeytop", "ModalKeytop", XtRBoolean, sizeof(Boolean),
     Offset(modal_keytop), XtRImmediate, (XtPointer)FALSE },
  { "modalThreshold", "ModalThreshold", XtRInt, sizeof(int),
     Offset(modal_threshold), XtRImmediate, (XtPointer)150 },
  { "keypad", "Keypad", XtRBoolean, sizeof(Boolean),
     Offset(keypad), XtRImmediate, (XtPointer)TRUE },
  { "functionkey", "FunctionKey", XtRBoolean, sizeof(Boolean),
     Offset(function_key), XtRImmediate, (XtPointer)TRUE },
  { "compact", "Compact", XtRBoolean, sizeof(Boolean),
     Offset(compact), XtRImmediate, (XtPointer)FALSE },
  { "keypadOnly", "KeypadOnly", XtRBoolean, sizeof(Boolean),
     Offset(keypad_only), XtRImmediate, (XtPointer)FALSE },
  { "keypadKeysym", "KeypadKeysym", XtRBoolean, sizeof(Boolean),
     Offset(keypad_keysym), XtRImmediate, (XtPointer)FALSE },
  { "autoAddKeysym", "AutoAddKeysym", XtRBoolean, sizeof(Boolean),
     Offset(auto_add_keysym), XtRImmediate, (XtPointer)TRUE },
  { "listWidgets", "Debug", XtRBoolean, sizeof(Boolean),
     Offset(list_widgets), XtRImmediate, (XtPointer)FALSE },
  { "positiveModifiers", "PositiveModifiers", XtRString, sizeof(char *),
    Offset(positive_modifiers), XtRImmediate, "" },
  { "text", "Text", XtRString, sizeof(char *),
    Offset(text), XtRImmediate, "" },
  { "file", "File", XtRString, sizeof(char *),
    Offset(file), XtRImmediate, "" },
  { "window", "Window", XtRString, sizeof(char *),
    Offset(window), XtRImmediate, "" },
  { "instance", "Instance", XtRString, sizeof(char *),
    Offset(instance), XtRImmediate, "" },
  { "widget", "Widget", XtRString, sizeof(char *),
    Offset(widget), XtRImmediate, "" },
  { "generalFont", XtCFont, XtRFontStruct, sizeof(XFontStruct *),
      Offset(general_font), XtRString, XtDefaultFont},
  { "letterFont", XtCFont, XtRFontStruct, sizeof(XFontStruct *),
      Offset(letter_font), XtRString, XtDefaultFont},
  { "specialFont", XtCFont, XtRFontStruct, sizeof(XFontStruct *),
      Offset(special_font), XtRString, XtDefaultFont},
  { "keypadFont", XtCFont, XtRFontStruct, sizeof(XFontStruct *),
      Offset(keypad_font), XtRString, XtDefaultFont},
  { "generalBackground", XtCBackground, XtRPixel, sizeof(Pixel),
     Offset(general_background), XtRString, "gray" },
  { "specialBackground", XtCBackground, XtRPixel, sizeof(Pixel),
     Offset(special_background), XtRString, "gray" },
  { "specialForeground", XtCForeground, XtRPixel, sizeof(Pixel),
     Offset(special_foreground), XtRString, "black" },
#ifdef USE_I18N
  { "specialFontSet", XtCFontSet, XtRFontSet, sizeof(XFontSet),
      Offset(special_fontset), XtRString, XtDefaultFontSet},
#endif
  { "highlightBackground", XtCBackground, XtRPixel, sizeof(Pixel),
     Offset(highlight_background), XtRString, "gray" },
  { "highlightForeground", XtCForeground, XtRPixel, sizeof(Pixel),
     Offset(highlight_foreground), XtRString, "forestgreen" },
  { "focusBackground", XtCBackground, XtRPixel, sizeof(Pixel),
     Offset(focus_background), XtRString, "gray" },
  { "remoteFocusBackground", XtCBackground, XtRPixel, sizeof(Pixel),
     Offset(remote_focus_background), XtRString, "cyan" },
  { "balloonBackground", XtCBackground, XtRPixel, sizeof(Pixel),
     Offset(balloon_background), XtRString, "LightYellow1" },
  { "launchBalloonBackground", XtCBackground, XtRPixel, sizeof(Pixel),
     Offset(launch_balloon_background), XtRString, "SkyBlue1" },

  { "normalkeys", "NormalKeys", XtRString, sizeof(char *),
    Offset(keys_normal), XtRImmediate, "" },
  { "shiftkeys", "ShiftKeys", XtRString, sizeof(char *),
    Offset(keys_shift), XtRImmediate, "" },
  { "altgrkeys", "AltgrKeys", XtRString, sizeof(char *),
    Offset(keys_altgr), XtRImmediate, "" },
  { "shiftaltgrkeys", "ShiftAltgrKeys", XtRString, sizeof(char *),
    Offset(keys_shift_altgr), XtRImmediate, "" },
  { "keylabels", "KeyLabels", XtRString, sizeof(char *),
    Offset(key_labels), XtRImmediate, "" },
  { "normalkeylabels", "NormalKeyLabels", XtRString, sizeof(char *),
    Offset(normal_key_labels), XtRImmediate, "" },
  { "shiftkeylabels", "ShiftKeyLabels", XtRString, sizeof(char *),
    Offset(shift_key_labels), XtRImmediate, "" },
  { "altgrkeylabels", "AltgrKeyLabels", XtRString, sizeof(char *),
    Offset(altgr_key_labels), XtRImmediate, "" },
  { "shiftaltgrkeylabels", "ShiftAltgrKeyLabels", XtRString, sizeof(char *),
    Offset(shift_altgr_key_labels), XtRImmediate, "" },

  { "normalkeypad", "NormalKeypad", XtRString, sizeof(char *),
    Offset(keypad_normal), XtRImmediate, "" },
  { "shiftkeypad", "ShiftKeypad", XtRString, sizeof(char *),
    Offset(keypad_shift), XtRImmediate, "" },
  { "keypad_labels", "KeypadLabels", XtRString, sizeof(char *),
    Offset(keypad_labels), XtRImmediate, "" },

  { "deadkeys", "DeadKeys", XtRString, sizeof(char *),
    Offset(deadkeys), XtRImmediate, "" },
  { "altgrKeycode", "AltgrKeycode", XtRInt, sizeof(int),
    Offset(altgr_keycode), XtRImmediate, (XtPointer)0 },

  { "keyFile", "KeyFile", XtRString, sizeof(char *),
    Offset(key_file), XtRImmediate, ".xvkbd" },
  { "dictFile", "DictFile", XtRString, sizeof(char *),
    Offset(dict_file), XtRImmediate, "/usr/share/dict/words" },
  { "customizations", "Customizations", XtRString, sizeof(char *),
    Offset(customizations), XtRImmediate, "default" },
  { "editableFunctionKeys", "EditableFunctionKeys", XtRInt, sizeof(int),
     Offset(editable_function_keys), XtRImmediate, (XtPointer)12 },

  { "maxWidthRatio", "MaxRatio", XtRFloat, sizeof(float),
     Offset(max_width_ratio), XtRString, "0.9" },
  { "maxHeightRatio", "MaxRatio", XtRFloat, sizeof(float),
     Offset(max_height_ratio), XtRString, "0.5" },
  { "textDelay", "TextDelay", XtRInt, sizeof(int),
     Offset(text_delay), XtRImmediate, (XtPointer)0 },

  { "keyClickPitch", "KeyClickPitch", XtRInt, sizeof(int),
     Offset(key_click_pitch), XtRImmediate, (XtPointer)1000 },
  { "keyClickDuration", "KeyClickDuration", XtRInt, sizeof(int),
     Offset(key_click_duration), XtRImmediate, (XtPointer)1 },
  { "autoClickDelay", "AutoClickDelay", XtRInt, sizeof(int),
     Offset(autoclick_delay), XtRImmediate, (XtPointer)0 },
};
#undef Offset

static XrmOptionDescRec options[] = {
  { "-geometry", ".windowGeometry", XrmoptionSepArg, NULL },
  { "-windowgeometry", ".windowGeometry", XrmoptionSepArg, NULL },
  { "-debug", ".debug", XrmoptionNoArg, "True" },
#ifdef USE_XTEST
  { "-xtest", ".xtest", XrmoptionNoArg, "True" },
  { "-xsendevent", ".xtest", XrmoptionNoArg, "False" },
  { "-no-jump-pointer", ".jumpPointer", XrmoptionNoArg, "False" },
  { "-no-back-pointer", ".jumpPointerBack", XrmoptionNoArg, "False" },
#endif
  { "-no-sync", ".noSync", XrmoptionNoArg, "True" },
  { "-always-on-top", ".alwaysOnTop", XrmoptionNoArg, "True" },  /* EXPERIMENTAL */
  { "-quick", ".quickModifiers", XrmoptionNoArg, "True" },
  { "-modifiers", ".positiveModifiers", XrmoptionSepArg, NULL },
  { "-text", ".text", XrmoptionSepArg, NULL },
  { "-file", ".file", XrmoptionSepArg, NULL },
  { "-delay", ".textDelay", XrmoptionSepArg, NULL },
  { "-window", ".window", XrmoptionSepArg, NULL },
  { "-instance", ".instance", XrmoptionSepArg, NULL },
  { "-widget", ".widget", XrmoptionSepArg, NULL },
  { "-altgr-lock", ".altgrLock", XrmoptionNoArg, "True" },
  { "-no-altgr-lock", ".altgrLock", XrmoptionNoArg, "False" },
  { "-no-repeat", ".autoRepeat", XrmoptionNoArg, "False" },
  { "-norepeat", ".autoRepeat", XrmoptionNoArg, "False" },
  { "-no-keypad", ".keypad", XrmoptionNoArg, "False" },
  { "-nokeypad", ".keypad", XrmoptionNoArg, "False" },
  { "-no-functionkey", ".functionkey", XrmoptionNoArg, "False" },
  { "-nofunctionkey", ".functionkey", XrmoptionNoArg, "False" },
  { "-highlight", ".highlightForeground", XrmoptionSepArg, NULL },
  { "-compact", ".compact", XrmoptionNoArg, "True" },
  { "-keypad", ".keypadOnly", XrmoptionNoArg, "True" },
  { "-true-keypad", ".keypadKeysym", XrmoptionNoArg, "True" },
  { "-truekeypad", ".keypadKeysym", XrmoptionNoArg, "True" },
  { "-no-add-keysym", ".autoAddKeysym", XrmoptionNoArg, "False" },
  { "-altgr-keycode", ".altgrKeycode", XrmoptionSepArg, NULL },
  { "-list", ".listWidgets", XrmoptionNoArg, "True" },
  { "-modal", ".modalKeytop", XrmoptionNoArg, "True" },
  { "-minimizable", ".minimizable", XrmoptionNoArg, "True" },
  { "-secure", ".secure", XrmoptionNoArg, "True" },
  { "-no_root", ".no_root", XrmoptionNoArg, "True" },
  { "-wait_idle", ".wait_idle", XrmoptionSepArg, NULL },
  { "-nonexitable", ".nonexitable", XrmoptionNoArg, "True" },
  { "-xdm", ".Secure", XrmoptionNoArg, "True" },
  { "-dict", ".dictFile", XrmoptionSepArg, NULL },
  { "-keyfile", ".keyFile", XrmoptionSepArg, NULL },
  { "-customizations", ".customizations", XrmoptionSepArg, NULL },
  { "-version", ".version", XrmoptionNoArg, "True" },
  { "-help", ".version", XrmoptionNoArg, "True" },
};

/*
 * Global variables
 */
static char dict_filename[PATH_MAX] = "";

static int argc1;
static char **argv1;

static XtAppContext app_con;
static Widget toplevel;
static Widget key_widgets[NUM_KEY_ROWS][NUM_KEY_COLS];
static Widget main_menu = None;

static Dimension toplevel_height = 1000;

static Display *dpy;
static Atom wm_delete_window = None;

static KeySym *keysym_table = NULL;
static int min_keycode, max_keycode;
static int keysym_per_keycode;
static Boolean error_detected;

static int alt_mask = 0;
static int meta_mask = 0;
static int altgr_mask = 0;
static KeySym altgr_keysym = NoSymbol;

static int shift_state = 0;
static int mouse_shift = 0;

static Display *target_dpy = NULL;

static Window toplevel_parent = None;
static Window focused_window = None;
static Window focused_subwindow = None;

static Pixmap xvkbd_pixmap = None;

static int AddKeysym(KeySym keysym, Boolean top);  /* forward */
static void SendString(const unsigned char *str);
static void MakeKeyboard(Boolean remake);
static void MakeKeypad(Widget form, Widget from_vert, Widget from_horiz);
static void MakeSunFunctionKey(Widget form, Widget from_vert, Widget from_horiz);
static void MakeDeadkeyPanel(Widget form);
static void RefreshMainMenu(void);
static void PopupFunctionKeyEditor(void);
static void DeleteWindowProc(Widget w, XEvent *event, String *pars, Cardinal *n_pars);

/*
 * Search for window which has specified instance name (WM_NAME)
 * or class name (WM_CLASS).
 */
static Window FindWindow(Window top, char *name)
{
  Window w;
  Window *children, dummy;
  unsigned int nchildren;
  int i;
  XClassHint hint;
  char *win_name;

  w = None;

  if (appres.debug) fprintf(stderr, "FindWindow: id=0x%lX", (long)top);

  if (XGetClassHint(target_dpy, top, &hint)) {
    if (hint.res_name) {
      if (appres.debug) fprintf(stderr, " instance=\"%s\"", hint.res_name);
      if ((strlen(name) > 0 && fnmatch(name, hint.res_name, 0) == 0)
	  || (strlen(appres.instance) > 0 && fnmatch(appres.instance, hint.res_name, 0) == 0)) w = top;
      XFree(hint.res_name);
    }
    if (strlen(name) > 0 && hint.res_class) {
      if (appres.debug) fprintf(stderr, " class=\"%s\"", hint.res_class);
      if (strlen(name) > 0 && fnmatch(name, hint.res_class, 0) == 0) w = top;
      XFree(hint.res_class);
    }
  }
  if (XFetchName(target_dpy, top, &win_name)) { /* window title */
    if (appres.debug) fprintf(stderr, " title=\"%s\"", win_name);
    if (strlen(name) > 0 && fnmatch(name, win_name, 0) == 0) w = top;
    XFree(win_name);
  }

  if (appres.debug) fprintf(stderr, "%s\n", (w == None) ? "" : " [matched]");

  if (w == None &&
      XQueryTree(target_dpy, top, &dummy, &dummy, &children, &nchildren)) {
    for (i = 0; i < nchildren; i++) {
      w = FindWindow(children[i], name);
      if (w != None) break;
    }
    if (children) XFree((char *)children);
  }

  return(w);
}

/*
 * This will be called to get window to set input focus,
 * when user pressed the "Focus" button.
 */
static void GetFocusedWindow(void)
{
  Cursor cursor;
  XEvent event;
  Window target_root, child;
  int junk_i;
  unsigned junk_u;
  Window junk_w;
  int scrn;
  int cur_x, cur_y, last_x, last_y;
  double x_ratio, y_ratio;

  XFlush(target_dpy);
  target_root = RootWindow(target_dpy, DefaultScreen(target_dpy));

  cursor = XCreateFontCursor(dpy, (target_dpy == dpy) ? XC_crosshair : XC_dot);
  if (XGrabPointer(dpy, RootWindow(dpy, DefaultScreen(dpy)), False, ButtonPressMask,
                   GrabModeSync, GrabModeAsync, None,
                   cursor, CurrentTime) == 0) {
    if (appres.debug)
      fprintf(stderr, "Grab pointer - waiting for button press\n");
    last_x = -1;
    last_y = -1;
    x_ratio = ((double)WidthOfScreen(DefaultScreenOfDisplay(target_dpy))
	       / WidthOfScreen(XtScreen(toplevel)));
    y_ratio = ((double)HeightOfScreen(DefaultScreenOfDisplay(target_dpy))
	       / HeightOfScreen(XtScreen(toplevel)));
    do {
      XAllowEvents(dpy, SyncPointer, CurrentTime);
      if (target_dpy == dpy) {
	XNextEvent(dpy, &event);
      } else {
	XCheckTypedEvent(dpy, ButtonPress, &event);
	if (XQueryPointer(dpy, RootWindow(dpy, DefaultScreen(dpy)), &junk_w, &junk_w,
			  &cur_x, &cur_y, &junk_i, &junk_i, &junk_u)) {
	  cur_x = cur_x * x_ratio;
	  cur_y = cur_y * y_ratio;
	}
	if (cur_x != last_x || cur_y != last_y) {
	  if (appres.debug) fprintf(stderr, "Moving pointer to (%d, %d) on %s\n",
				    cur_x, cur_y, XDisplayString(target_dpy));
	  XWarpPointer(target_dpy, None, target_root, 0, 0, 0, 0, cur_x, cur_y);
	  XFlush(target_dpy);
	  last_x = cur_x;
	  last_y = cur_y;
	  XQueryPointer(target_dpy, target_root, &junk_w, &child,
			&cur_x, &cur_y, &junk_i, &junk_i, &junk_u);
	  usleep(10000);
	} else {
	  usleep(100000);
	}
      }
    } while (event.type != ButtonPress);
    XUngrabPointer(dpy, CurrentTime);

    focused_window = None;
    if (target_dpy == dpy) focused_window = event.xbutton.subwindow;
    if (focused_window == None) {
      XFlush(target_dpy);
      for (scrn = 0; scrn < ScreenCount(target_dpy); scrn++) {
	if (XQueryPointer(target_dpy, RootWindow(target_dpy, scrn), &junk_w, &child,
			  &junk_i, &junk_i, &junk_i, &junk_i, &junk_u)) {
	  if (appres.debug)
	    fprintf(stderr, "Window on the other display/screen (screen #%d of %s) focused\n",
		    scrn, XDisplayString(target_dpy));
	  target_root = RootWindow(target_dpy, scrn);
	  focused_window = child;
	  break;
	}
      }
    }
    if (focused_window == None) focused_window = target_root;
    else focused_window = XmuClientWindow(target_dpy, focused_window);
    if (appres.debug) fprintf(stderr, "Selected window is: 0x%lX on %s\n",
			      focused_window, XDisplayString(target_dpy));

    if (target_dpy == dpy && XtWindow(toplevel) == focused_window) {
      focused_window = None;
      focused_subwindow = focused_window;
      return;
    }

    focused_subwindow = focused_window;
    do {  /* search the child window */
      XQueryPointer(target_dpy, focused_subwindow, &junk_w, &child,
                    &junk_i, &junk_i, &junk_i, &junk_i, &junk_u);
      if (child != None) {
        focused_subwindow = child;
        if (appres.debug) fprintf(stderr, "  going down: 0x%lX\n", focused_subwindow);
      }
    } while (child != None);
    if (appres.list_widgets || strlen(appres.widget) != 0) {
      child = FindWidget(toplevel, focused_window, appres.widget);
      if (child != None) focused_subwindow = child;
    }
  } else {
    fprintf(stderr, "%s: cannot grab pointer\n", PROGRAM_NAME);
  }
}

/*
 * Read keyboard mapping and modifier mapping.
 * Keyboard mapping is used to know what keys are in shifted position.
 * Modifier mapping is required because we should know Alt and Meta
 * key are used as which modifier.
 */
static void Highlight(char *name, Boolean state);

static void ReadKeymap(void)
{
  int i;
  int keycode, inx, pos;
  KeySym keysym;
  XModifierKeymap *modifiers;
  Widget w;
  int last_altgr_mask;

  XDisplayKeycodes(target_dpy, &min_keycode, &max_keycode);
  if (keysym_table != NULL) XFree(keysym_table);
  keysym_table = XGetKeyboardMapping(target_dpy,
                             min_keycode, max_keycode - min_keycode + 1,
                             &keysym_per_keycode);
  for (keycode = min_keycode; keycode <= max_keycode; keycode++) {
    /* if the first keysym is alphabet and the second keysym is NoSymbol,
       it is equivalent to pair of lowercase and uppercase alphabet */
    inx = (keycode - min_keycode) * keysym_per_keycode;
    if (keysym_table[inx + 1] == NoSymbol
	&& ((XK_A <= keysym_table[inx] && keysym_table[inx] <= XK_Z)
	    || (XK_a <= keysym_table[inx] && keysym_table[inx] <= XK_z))) {
      if (XK_A <= keysym_table[inx] && keysym_table[inx] <= XK_Z)
	keysym_table[inx] = keysym_table[inx] - XK_A + XK_a;
      keysym_table[inx + 1] = keysym_table[inx] - XK_a + XK_A;
    }
  }

  last_altgr_mask = altgr_mask;
  alt_mask = 0;
  meta_mask = 0;
  altgr_mask = 0;
  altgr_keysym = NoSymbol;
  modifiers = XGetModifierMapping(target_dpy);
  for (i = 0; i < 8; i++) {
    for (pos = 0; pos < modifiers->max_keypermod; pos++) {
      keycode = modifiers->modifiermap[i * modifiers->max_keypermod + pos];
      if (keycode < min_keycode || max_keycode < keycode) continue;

      keysym = keysym_table[(keycode - min_keycode) * keysym_per_keycode];
      if (keysym == XK_Alt_L || keysym == XK_Alt_R) {
	alt_mask = 1 << i;
      } else if (keysym == XK_Meta_L || keysym == XK_Meta_R) {
	meta_mask = 1 << i;
      } else if (keysym == XK_Mode_switch) {
	if (appres.debug)
	  fprintf(stderr, "%s: found Mode_switch at %dth modifier\n", PROGRAM_NAME, i);
	if (altgr_keysym == XK_ISO_Level3_Shift) {
	  if (appres.debug)
	    fprintf(stderr, "%s: both ISO_Level3_Shift and Mode_switch found - ISO_Level3_Shift prefered\n", PROGRAM_NAME);
	} else {
	  altgr_mask = 0x0101 << i;
	  /* I don't know why, but 0x2000 was required for mod3 on my Linux box */
	  altgr_keysym = keysym;
	}
      } else if (keysym == XK_ISO_Level3_Shift) {
	/* if no Mode_switch, try to use ISO_Level3_Shift instead */
	/* however, it may not work as intended - I don't know why */
	if (appres.debug)
	  fprintf(stderr, "%s: found ISO_Level3_Shift at %dth modifier\n", PROGRAM_NAME, i);
	if (altgr_keysym == XK_Mode_switch) {
	  if (appres.debug)
	    fprintf(stderr, "%s: both ISO_Level3_Shift and Mode_switch found - ISO_Level3_Shift prefered\n", PROGRAM_NAME);
	}
	altgr_mask = 1 << i;
	altgr_keysym = keysym;
      }
    }
  }
  XFreeModifiermap(modifiers);

  if (altgr_keysym != XK_Mode_switch) {
    fprintf(stderr, "%s: Mode_switch not available as a modifier\n", PROGRAM_NAME);
    if (altgr_keysym == XK_ISO_Level3_Shift)
      fprintf(stderr, "%s: although ISO_Level3_Shift is used instead, AltGr may not work correctly\n", PROGRAM_NAME);
    else
      fprintf(stderr, "%s: AltGr can't be used\n", PROGRAM_NAME);
  }

  w = XtNameToWidget(toplevel, "*Multi_key");
  if (w != None) {
    if (XKeysymToKeycode(target_dpy, XK_Multi_key) == NoSymbol) {
      if (!appres.auto_add_keysym || AddKeysym(XK_Multi_key, FALSE) == NoSymbol)
	XtSetSensitive(w, FALSE);
    }
  }
  w = XtNameToWidget(toplevel, "*Mode_switch");
  if (w != None) {
    if (appres.xtest && 0 < appres.altgr_keycode) {
      XtSetSensitive(w, TRUE);
      if (appres.debug)
	fprintf(stderr, "%s: keycode %d will be used for AltGr - it was specified with altgrKeycode\n",
		PROGRAM_NAME, appres.altgr_keycode);
    } else if (altgr_mask) {
      XtSetSensitive(w, TRUE);
    } else {
      XtSetSensitive(w, FALSE);
      if (shift_state & last_altgr_mask) {
	shift_state &= ~last_altgr_mask;
	Highlight("Mode_switch", FALSE);
      }
    }
  }
}

/*
 * This will called when X error is detected when attempting to
 * send a event to a client window;  this will normally caused
 * when the client window is destroyed.
 */
static int MyErrorHandler(Display *my_dpy, XErrorEvent *event)
{
  char msg[200];

  error_detected = TRUE;
  if (event->error_code == BadWindow) {
    if (appres.debug)
      fprintf(stderr, "%s: BadWindow - couldn't find target window 0x%lX (destroyed?)\n",
	      PROGRAM_NAME, (long)focused_window);
    return 0;
  }
  XGetErrorText(my_dpy, event->error_code, msg, sizeof(msg) - 1);
  fprintf(stderr, "X error trapped: %s, request-code=%d\n", msg, event->request_code);
  return 0;
}

/*
 * Send event to the focused window.
 * If input focus is specified explicitly, select the window
 * before send event to the window.
 */
static void SendEvent(XKeyEvent *event)
{
  static Boolean first = TRUE;

  if (!appres.no_sync) {
    XSync(event->display, FALSE);
    XSetErrorHandler(MyErrorHandler);
  }

  error_detected = FALSE;
  if (focused_window != None) {
    /* set input focus if input focus is set explicitly */
    if (appres.debug)
      fprintf(stderr, "Set input focus to window 0x%lX (0x%lX)\n",
              (long)focused_window, (long)event->window);
    XSetInputFocus(event->display, focused_window, RevertToParent, CurrentTime);
    if (!appres.no_sync) XSync(event->display, FALSE);
  }
  if (!error_detected) {
    if (appres.xtest) {
#ifdef USE_XTEST
      if (appres.debug)
	fprintf(stderr, "XTestFakeKeyEvent(0x%lx, %ld, %d)\n",
		(long)event->display, (long)event->keycode, event->type == KeyPress);
      if (appres.jump_pointer) {
	Window root, child, w;
	int root_x, root_y, x, y;
	unsigned int mask;
	int revert_to;

	w = None;
	if (first || strlen(appres.text) == 0 || appres.jump_pointer_back) {
	  first = FALSE;

	  w = focused_subwindow;
	  if (w == None && appres.jump_pointer_always)
	    XGetInputFocus(event->display, &w, &revert_to);

	  if (w != None) {
	    if (appres.debug)
	      fprintf(stderr, "SendEvent: jump pointer to window 0x%lx\n", (long)w);

	    XQueryPointer(event->display, w,
			  &root, &child, &root_x, &root_y, &x, &y, &mask);
	    XWarpPointer(event->display, None, w, 0, 0, 0, 0, 1, 1);
	    XFlush(event->display);
	  }
	}

	XTestFakeKeyEvent(event->display, event->keycode, event->type == KeyPress, 0);
	XFlush(event->display);

	if (w != None && appres.jump_pointer_back) {
	  XWarpPointer(event->display, None, root, 0, 0, 0, 0, root_x, root_y);
	  XFlush(event->display);
	}
      } else {
	XTestFakeKeyEvent(event->display, event->keycode, event->type == KeyPress, 0);
 	XFlush(event->display);
      }
#else
      fprintf(stderr, "%s: this binary is compiled without XTEST support\n",
	      PROGRAM_NAME);
#endif
    } else {
      XSendEvent(event->display, event->window, TRUE, KeyPressMask, (XEvent *)event);
      if (!appres.no_sync) XSync(event->display, FALSE);

      if (error_detected
	  && (focused_subwindow != None) && (focused_subwindow != event->window)) {
	error_detected = FALSE;
	event->window = focused_subwindow;
	if (appres.debug)
	  fprintf(stderr, "   retry: send event to window 0x%lX (0x%lX)\n",
		  (long)focused_window, (long)event->window);
	XSendEvent(event->display, event->window, TRUE, KeyPressMask, (XEvent *)event);
	if (!appres.no_sync) XSync(event->display, FALSE);
      }
    }
  }

  if (error_detected) {
    /* reset focus because focused window is (probably) no longer exist */
    XBell(dpy, 0);
    focused_window = None;
    focused_subwindow = None;
  }

  XSetErrorHandler(NULL);
}

/*
 * Insert a specified keysym to unused position in the keymap table.
 * This will be called to add required keysyms on-the-fly.
 * if the second parameter is TRUE, the keysym will be added to the
 * non-shifted position - this may be required for modifier keys
 * (e.g. Mode_switch) and some special keys (e.g. F20).
 */
static int AddKeysym(KeySym keysym, Boolean top)
{
  int keycode, pos, max_pos, inx, phase;

  if (top) {
    max_pos = 0;
  } else {
    max_pos = keysym_per_keycode - 1;
    if (4 <= max_pos) max_pos = 3;
    if (2 <= max_pos && altgr_keysym != XK_Mode_switch) max_pos = 1;
  }

  for (phase = 0; phase < 2; phase++) {
    for (keycode = max_keycode; min_keycode <= keycode; keycode--) {
      for (pos = max_pos; 0 <= pos; pos--) {
	inx = (keycode - min_keycode) * keysym_per_keycode;
	if ((phase != 0 || keysym_table[inx] == NoSymbol) && keysym_table[inx] < 0xFF00) {
	  /* In the first phase, to avoid modifing existing keys, */
	  /* add the keysym only to the keys which has no keysym in the first position. */
	  /* If no place fuond in the first phase, add the keysym for any keys except */
	  /* for modifier keys and other special keys */
	  if (keysym_table[inx + pos] == NoSymbol) {
	    if (appres.debug)
	      fprintf(stderr, "*** Adding keysym \"%s\" at keycode %d position %d/%d\n",
		      XKeysymToString(keysym), keycode, pos, keysym_per_keycode);
	    keysym_table[inx + pos] = keysym;
	    XChangeKeyboardMapping(target_dpy, keycode, keysym_per_keycode, &keysym_table[inx], 1);
	    XFlush(target_dpy);
	    return keycode;
	  }
	}
      }
    }
  }
  fprintf(stderr, "%s: couldn't add \"%s\" to keymap\n",
	  PROGRAM_NAME, XKeysymToString(keysym));
  XBell(dpy, 0);
  return NoSymbol;
}

/*
 * Add the specified key as a new modifier.
 * This is used to use Mode_switch (AltGr) as a modifier.
 */
static void AddModifier(KeySym keysym)
{
  XModifierKeymap *modifiers;
  int keycode, i, pos;

  keycode = XKeysymToKeycode(target_dpy, keysym);
  if (keycode == NoSymbol) keycode = AddKeysym(keysym, TRUE);
  
  modifiers = XGetModifierMapping(target_dpy);
  for (i = 7; 3 < i; i--) {
    if (modifiers->modifiermap[i * modifiers->max_keypermod] == NoSymbol
	|| ((keysym_table[(modifiers->modifiermap[i * modifiers->max_keypermod]
			   - min_keycode) * keysym_per_keycode]) == XK_ISO_Level3_Shift
	    && keysym == XK_Mode_switch)) {
      for (pos = 0; pos < modifiers->max_keypermod; pos++) {
	if (modifiers->modifiermap[i * modifiers->max_keypermod + pos] == NoSymbol) {
	  if (appres.debug)
	    fprintf(stderr, "Adding modifier \"%s\" as %dth modifier\n",
		    XKeysymToString(keysym), i);
	  modifiers->modifiermap[i * modifiers->max_keypermod + pos] = keycode;
	  XSetModifierMapping(target_dpy, modifiers);
	  return;
	}
      }
    }
  }
  fprintf(stderr, "%s: couldn't add \"%s\" as modifier\n",
	  PROGRAM_NAME, XKeysymToString(keysym));
  XBell(dpy, 0);
}

/*
 * Send sequence of KeyPressed/KeyReleased events to the focused
 * window to simulate keyboard.  If modifiers (shift, control, etc)
 * are set ON, many events will be sent.
 */
static void SendKeyPressedEvent(KeySym keysym, unsigned int shift)
{
  Window cur_focus;
  int revert_to;
  XKeyEvent event;
  int keycode;
  Window root, *children;
  unsigned int n_children;
  int phase, inx;
  Boolean found;

  if (focused_subwindow != None)
    cur_focus = focused_subwindow;
  else
    XGetInputFocus(target_dpy, &cur_focus, &revert_to);

  if (appres.debug) {
    char ch = '?';
    if ((keysym & ~0x7f) == 0 && isprint(keysym)) ch = keysym;
    fprintf(stderr, "SendKeyPressedEvent: focus=0x%lX, key=0x%lX (%c), shift=0x%lX\n",
            (long)cur_focus, (long)keysym, ch, (long)shift);
  }

  if (XtWindow(toplevel) != None) {
    if (toplevel_parent == None) {
      XQueryTree(target_dpy, RootWindow(target_dpy, DefaultScreen(target_dpy)),
                 &root, &toplevel_parent, &children, &n_children);
      XFree(children);
    }
    if (cur_focus == None || cur_focus == PointerRoot
	|| cur_focus == XtWindow(toplevel) || cur_focus == toplevel_parent) {
      /* notice user when no window focused or the xvkbd window is focused */
      XBell(dpy, 0);
      return;
    }
  }

  found = FALSE;
  keycode = 0;
  if (keysym != NoSymbol) {
    for (phase = 0; phase < 2; phase++) {
      for (keycode = min_keycode; !found && (keycode <= max_keycode); keycode++) {
	/* Determine keycode for the keysym:  we use this instead
	   of XKeysymToKeycode() because we must know shift_state, too */
	inx = (keycode - min_keycode) * keysym_per_keycode;
	if (keysym_table[inx] == keysym) {
	  shift &= ~altgr_mask;
	  if (keysym_table[inx + 1] != NoSymbol) shift &= ~ShiftMask;
	  found = TRUE;
	  break;
	} else if (keysym_table[inx + 1] == keysym) {
	  shift &= ~altgr_mask;
	  shift |= ShiftMask;
	  found = TRUE;
	  break;
	}
      }
      if (!found && altgr_mask && 3 <= keysym_per_keycode) {
	for (keycode = min_keycode; !found && (keycode <= max_keycode); keycode++) {
	  inx = (keycode - min_keycode) * keysym_per_keycode;
	  if (keysym_table[inx + 2] == keysym) {
	    shift &= ~ShiftMask;
	    shift |= altgr_mask;
	    found = TRUE;
	    break;
	  } else if (4 <= keysym_per_keycode && keysym_table[inx + 3] == keysym) {
	    shift |= ShiftMask | altgr_mask;
	    found = TRUE;
	    break;
	  }
	}
      }
      if (found || !appres.auto_add_keysym) break;

      if (0xF000 <= keysym) {
	/* for special keys such as function keys,
	   first try to add it in the non-shifted position of the keymap */
	if (AddKeysym(keysym, TRUE) == NoSymbol) AddKeysym(keysym, FALSE);
      } else {
	AddKeysym(keysym, FALSE);
      }
    }
    if (appres.debug) {
      if (found)
	fprintf(stderr, "SendKeyPressedEvent: keysym=0x%lx, keycode=%ld, shift=0x%lX\n",
		(long)keysym, (long)keycode, (long)shift);
      else
	fprintf(stderr, "SendKeyPressedEvent: keysym=0x%lx - keycode not found\n",
		(long)keysym);
    }
  }

  event.display = target_dpy;
  event.window = cur_focus;
  event.root = RootWindow(event.display, DefaultScreen(event.display));
  event.subwindow = None;
  event.time = CurrentTime;
  event.x = 1;
  event.y = 1;
  event.x_root = 1;
  event.y_root = 1;
  event.same_screen = TRUE;

#ifdef USE_XTEST
  if (appres.xtest) {
    Window root, child;
    int root_x, root_y, x, y;
    unsigned int mask;

    XQueryPointer(target_dpy, event.root, &root, &child, &root_x, &root_y, &x, &y, &mask);

    event.type = KeyRelease;
    event.state = 0;
    if (mask & ControlMask) {
      event.keycode = XKeysymToKeycode(target_dpy, XK_Control_L);
      SendEvent(&event);
    }
    if (mask & alt_mask) {
      event.keycode = XKeysymToKeycode(target_dpy, XK_Alt_L);
      SendEvent(&event);
    }
    if (mask & meta_mask) {
      event.keycode = XKeysymToKeycode(target_dpy, XK_Meta_L);
      SendEvent(&event);
    }
    if (mask & altgr_mask) {
      if (0 < appres.altgr_keycode)
	event.keycode = appres.altgr_keycode;
      else
	event.keycode = XKeysymToKeycode(target_dpy, altgr_keysym);
      SendEvent(&event);
    }
    if (mask & ShiftMask) {
      event.keycode = XKeysymToKeycode(target_dpy, XK_Shift_L);
      SendEvent(&event);
    }
    if (mask & LockMask) {
      event.keycode = XKeysymToKeycode(target_dpy, XK_Caps_Lock);
      SendEvent(&event);
    }
  }
#endif

  event.type = KeyPress;
  event.state = 0;
  if (shift & ControlMask) {
    event.keycode = XKeysymToKeycode(target_dpy, XK_Control_L);
    SendEvent(&event);
    event.state |= ControlMask;
  }
  if (shift & alt_mask) {
    event.keycode = XKeysymToKeycode(target_dpy, XK_Alt_L);
    SendEvent(&event);
    event.state |= alt_mask;
  }
  if (shift & meta_mask) {
    event.keycode = XKeysymToKeycode(target_dpy, XK_Meta_L);
    SendEvent(&event);
    event.state |= meta_mask;
  }
  if (shift & altgr_mask) {
    if (0 < appres.altgr_keycode)
      event.keycode = appres.altgr_keycode;
    else
      event.keycode = XKeysymToKeycode(target_dpy, altgr_keysym);
    SendEvent(&event);
    event.state |= altgr_mask;
  }
  if (shift & ShiftMask) {
    event.keycode = XKeysymToKeycode(target_dpy, XK_Shift_L);
    SendEvent(&event);
    event.state |= ShiftMask;
  }

  if (keysym != NoSymbol) {  /* send event for the key itself */
    event.keycode = found ? keycode : XKeysymToKeycode(target_dpy, keysym);
    if (event.keycode == NoSymbol) {
      if ((keysym & ~0x7f) == 0 && isprint(keysym))
        fprintf(stderr, "%s: no such key: %c\n",
                PROGRAM_NAME, (char)keysym);
      else if (XKeysymToString(keysym) != NULL)
        fprintf(stderr, "%s: no such key: keysym=%s (0x%lX)\n",
                PROGRAM_NAME, XKeysymToString(keysym), (long)keysym);
      else
        fprintf(stderr, "%s: no such key: keysym=0x%lX\n",
                PROGRAM_NAME, (long)keysym);
      XBell(dpy, 0);
    } else {
      SendEvent(&event);
      event.type = KeyRelease;
      SendEvent(&event);
    }
  }

  event.type = KeyRelease;
  if (shift & ShiftMask) {
    event.keycode = XKeysymToKeycode(target_dpy, XK_Shift_L);
    SendEvent(&event);
    event.state &= ~ShiftMask;
  }
  if (shift & altgr_mask) {
    if (0 < appres.altgr_keycode)
      event.keycode = appres.altgr_keycode;
    else
      event.keycode = XKeysymToKeycode(target_dpy, altgr_keysym);
    SendEvent(&event);
    event.state &= ~altgr_mask;
  }
  if (shift & meta_mask) {
    event.keycode = XKeysymToKeycode(target_dpy, XK_Meta_L);
    SendEvent(&event);
    event.state &= ~meta_mask;
  }
  if (shift & alt_mask) {
    event.keycode = XKeysymToKeycode(target_dpy, XK_Alt_L);
    SendEvent(&event);
    event.state &= ~alt_mask;
  }
  if (shift & ControlMask) {
    event.keycode = XKeysymToKeycode(target_dpy, XK_Control_L);
    SendEvent(&event);
    event.state &= ~ControlMask;
  }

  if (appres.no_sync) XFlush(dpy);
}

/*
 * Word completion - list of words which match the prefix entered
 * via xvkbd can be listed, and choosing one of them will send the
 * suffix to the clients.
 * Words for completion will be read from dictionary file specified
 * with xvkbd.dictFile resource, such as /usr/dict/words.
 */
static Widget completion_panel = None;
static Widget completion_entry = None;
static Widget completion_list = None;

static Widget props_dict_entry = None;

static char completion_text[100] = "";

#define HASH_SIZE 100

#define Hash(str)   ((toupper(str[0]) * 26 + toupper(str[1])) % HASH_SIZE)

static struct WORDLIST {
  struct WORDLIST *next;
  char *word;
} completion_words[HASH_SIZE];
static int n_completion_words = 0;

#define MAX_WORDS 200

static String word_list[MAX_WORDS + 1];
static int n_word_list = 0;

static void SetDefaultDictionary(void)
{
  strncpy(dict_filename, appres.dict_file, sizeof(dict_filename));
  XtVaSetValues(props_dict_entry, XtNstring, dict_filename, NULL);
}

static void ReadCompletionDictionary(void)
{
  static Boolean first = TRUE;
  static char cur_dict_filename[PATH_MAX] = "";
  FILE *fp;
  struct WORDLIST *node_ptr;
  char str[50];
  int i;
  struct WORDLIST *p;

  if (strcmp(cur_dict_filename, dict_filename) == 0) return;
  strcpy(cur_dict_filename, dict_filename);

  if (!first) {
    int cnt = 0;
    for (i = 0; i < HASH_SIZE; i++) {
      while (completion_words[i].next != NULL) {
	p = completion_words[i].next;
	completion_words[i].next = p->next;
	free(p);
	cnt++;
      }
    }
    if (appres.debug)
      fprintf(stderr, "ReadCompletionDictionary: %d words freed\n", cnt);
  }
  first = FALSE;

  for (i = 0; i < HASH_SIZE; i++) {
    completion_words[i].next = NULL;
    completion_words[i].word = NULL;
  }

  n_completion_words = 0;
  fp = fopen(dict_filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "%s: can't read dictionary file %s: %s\n",
	    PROGRAM_NAME, dict_filename, strerror(errno));
  } else {
    while (fgets(str, sizeof(str) - 1, fp)) {
      if (3 < strlen(str)) {
	str[strlen(str) - 1] = '\0';
	node_ptr = &completion_words[Hash(str)];
	while (node_ptr->word != NULL) node_ptr = node_ptr->next;

	node_ptr->word = XtNewString(str);
	node_ptr->next = malloc(sizeof(struct WORDLIST));
	node_ptr->next->next = NULL;
	node_ptr->next->word = NULL;
	n_completion_words++;
      }
    }
    fclose(fp);

    if (appres.debug)
      fprintf(stderr, "ReadCompletionDictionary: %d words allocated\n", n_completion_words);
  }
}

static void AddToCompletionText(KeySym keysym)
{
  int len;
  struct WORDLIST *node_ptr;

  if (completion_entry != None) {

    if (n_completion_words == 0) {
      XtVaSetValues(completion_entry, XtNlabel, "Couldn't read dictionary file", NULL);
      return;
    }

    len = strlen(completion_text);
    if (keysym == XK_BackSpace || keysym == XK_Delete) {
      if (0 < len) completion_text[len - 1] = '\0';
    } else if (keysym != NoSymbol
	       && !ispunct((char)keysym) && !isspace((char)keysym)) {
      if (len < sizeof(completion_text) - 2) {
	completion_text[len + 1] = '\0';
	completion_text[len] = keysym;
      }
    } else {
      completion_text[0] = '\0';
    }
    XtVaSetValues(completion_entry, XtNlabel, completion_text, NULL);

    n_word_list = 0;
    if (2 <= strlen(completion_text)) {
      node_ptr = &completion_words[Hash(completion_text)];
      while (node_ptr->word != NULL && n_word_list < MAX_WORDS) {
	if (strlen(completion_text) + 1 < strlen(node_ptr->word) &&
	    strncasecmp(node_ptr->word, completion_text, strlen(completion_text)) == 0) {
	  word_list[n_word_list] = node_ptr->word;
	  n_word_list = n_word_list + 1;
	}
	node_ptr = node_ptr->next;
      }
    }
    word_list[n_word_list] = NULL;
    XawListChange(completion_list, word_list, 0, 0, TRUE);
  }
}

static void CompletionWordSelected(Widget w, XtPointer client_data, XtPointer call_data)
{
  Boolean capitalize;
  unsigned char ch;
  int n, i;

  n = ((XawListReturnStruct *)call_data)->list_index;
  if (0 <= n && n < n_word_list) {
    capitalize = TRUE;
    for (i = 0; completion_text[i] != '\0'; i++) {
      if (islower(completion_text[i])) capitalize = FALSE;
    }
    for (i = strlen(completion_text); word_list[n][i] != '\0'; i++) {
      ch = word_list[n][i];
      if (capitalize) ch = toupper(ch);
      SendKeyPressedEvent(ch, 0);
    }
  }
  AddToCompletionText(NoSymbol);
}

static void PopupCompletionPanel(void)
{
  Widget form, label, view;
  char msg[100];

  if (completion_panel == None) {
    completion_panel = XtVaCreatePopupShell("completion_panel", transientShellWidgetClass,
					toplevel, NULL);
    form = XtVaCreateManagedWidget("form", formWidgetClass, completion_panel, NULL);
    label = XtVaCreateManagedWidget("label", labelWidgetClass, form, NULL);
    completion_entry = XtVaCreateManagedWidget("entry", labelWidgetClass, form,
				    XtNfromHoriz, label, NULL);
    view = XtVaCreateManagedWidget("view", viewportWidgetClass, form,
				    XtNfromVert, label, NULL);
    completion_list = XtVaCreateManagedWidget("list", listWidgetClass, view, NULL);
    XtAddCallback(completion_list, XtNcallback, CompletionWordSelected, NULL);
    XtRealizeWidget(completion_panel);
    XSetWMProtocols(dpy, XtWindow(completion_panel), &wm_delete_window, 1);

    XtPopup(completion_panel, XtGrabNone);
    AddToCompletionText(NoSymbol);
    XFlush(dpy);
  } else {
    XtPopup(completion_panel, XtGrabNone);
  }

  ReadCompletionDictionary();

  sprintf(msg, "%d words in the dictionary", n_completion_words);
  XtVaSetValues(completion_entry, XtNlabel, msg, NULL);

  completion_text[0] = '\0';
  n_word_list = 0;
  word_list[n_word_list] = NULL;
  XawListChange(completion_list, word_list, 0, 0, TRUE);
}

/*
 * Send given string to the focused window as if the string
 * is typed from a keyboard.
 */
static void KeyPressed(Widget w, char *key, char *data);

static void SendString(const unsigned char *str)
{
  const unsigned char *cp, *cp2;  /* I remember "unsigned" might be required for some systems */
  char key[50];
  int len;
  int val;
  Window target_root, child, junk_w;
  int junk_i;
  unsigned junk_u;
  int cur_x, cur_y;

  shift_state = 0;
  for (cp = str; *cp != '\0'; cp++) {
    if (0 < appres.text_delay)
      usleep(appres.text_delay * 1000);
    if (appres.wait_idle && strlen(appres.wait_idle) > 0) {
      int pid = atoi(appres.wait_idle);
      int ret;
      do {
	char cmd[80];
	snprintf(cmd, sizeof(cmd), "/proc/%d/status", pid);
	FILE *f = fopen(cmd, "r");
	if (f == NULL) {
	  fprintf(stderr, "Process not found: %d\n", pid);
	  exit(-1);
	}
	fclose(f);
	snprintf(cmd, sizeof(cmd), "grep 'State.*running' /proc/%d/status", pid);
	ret = system(cmd);
	if (ret == 0)
	  usleep(50);
      } while (ret == 0);
    }
    if (*cp == '\\') {
      cp++;
      switch (*cp) {
      case '\0':
        fprintf(stderr, "%s: missing character after \"\\\"\n",
                PROGRAM_NAME);
        return;
      case '[':  /* we can write any keysym as "\[the-keysym]" here */
        cp2 = strchr(cp, ']');
        if (cp2 == NULL) {
          fprintf(stderr, "%s: no closing \"]\" after \"\\[\"\n",
                  PROGRAM_NAME);
        } else {
          len = cp2 - cp - 1;
          if (sizeof(key) <= len) len = sizeof(key) - 1;
          strncpy(key, cp + 1, len);
          key[len] = '\0';
          KeyPressed(None, key, NULL);
          cp = cp2;
        }
        break;
      case 'S': shift_state |= ShiftMask; break;
      case 'C': shift_state |= ControlMask; break;
      case 'A': shift_state |= alt_mask; break;
      case 'M': shift_state |= meta_mask; break;
      case 'b': SendKeyPressedEvent(XK_BackSpace, shift_state); shift_state = 0; break;
      case 't': SendKeyPressedEvent(XK_Tab, shift_state); shift_state = 0; break;
      case 'n': SendKeyPressedEvent(XK_Linefeed, shift_state); shift_state = 0; break;
      case 'r': SendKeyPressedEvent(XK_Return, shift_state); shift_state = 0; break;
      case 'e': SendKeyPressedEvent(XK_Escape, shift_state); shift_state = 0; break;
      case 'd': SendKeyPressedEvent(XK_Delete, shift_state); shift_state = 0; break;
      case 'D':  /* delay */
	cp++;
	if ('1' <= *cp && *cp <= '9') {
	  usleep((*cp - '0') * 100000);
	} else {
          fprintf(stderr, "%s: no digit after \"\\m\"\n",
                  PROGRAM_NAME);
	}
	break;
      case 'm':  /* simulate click mouse button */
	cp++;
	if ('1' <= *cp && *cp <= '9') {
	  if (appres.debug) fprintf(stderr, "XTestFakeButtonEvent(%d)\n", *cp - '0');
	  XTestFakeButtonEvent(target_dpy, *cp - '0', True, CurrentTime);
	  XTestFakeButtonEvent(target_dpy, *cp - '0', False, CurrentTime);
	  XFlush(dpy);
	} else {
          fprintf(stderr, "%s: no digit after \"\\m\"\n",
                  PROGRAM_NAME);
	}
	break;
      case 'x':
      case 'y':  /* move mouse pointer */
	sscanf(cp + 1, "%d", &val);
	target_root = RootWindow(target_dpy, DefaultScreen(target_dpy));
	XQueryPointer(target_dpy, target_root, &junk_w, &child,
		      &cur_x, &cur_y, &junk_i, &junk_i, &junk_u);
	if (*cp == 'x') {
	  if (isdigit(*(cp + 1))) cur_x = val;
	  else cur_x += val;
	} else {
	  if (isdigit(*(cp + 1))) cur_y = val;
	  else cur_y += val;
	}
	XWarpPointer(target_dpy, None, target_root, 0, 0, 0, 0, cur_x, cur_y);
	XFlush(dpy);
	cp++;
	while (isdigit(*(cp + 1)) || *(cp + 1) == '+' || *(cp + 1) == '-') cp++;
        break;
      default:
	SendKeyPressedEvent(*cp, shift_state);
	shift_state = 0;
	break;
      }
    } else {
      SendKeyPressedEvent(*cp, shift_state);
      shift_state = 0;
    }
  }
}

/*
 * Send content of the file as if the it is typed from a keyboard.
 */
static void SendFileContent(const char *file)
{
  FILE *fp;
  int ch;

  fp = stdin;
  if (strcmp(file, "-") != 0) fp = fopen(file, "r");

  if (fp == NULL) {
    fprintf(stderr, "%s: can't read the file: %s\n", PROGRAM_NAME, file);
    exit(1);
  }
  while ((ch = fgetc(fp)) != EOF) {
    if (0 < appres.text_delay) usleep(appres.text_delay * 1000);
    if (ch == '\n') {  /* newline - send Return instead */
      SendKeyPressedEvent(XK_Return, 0);
    } else if (ch == '\033') {  /* ESC */
      SendKeyPressedEvent(XK_Escape, 0);
    } else if (ch == '\177') {  /* DEL */
      SendKeyPressedEvent(XK_Delete, 0);
    } else if (1 <= ch && ch <= 26) {  /* Ctrl-x */
      SendKeyPressedEvent('a' + ch - 1, ControlMask);
    } else {  /* normal characters */
      SendKeyPressedEvent(ch, 0);
    }
  }
  if (strcmp(file, "-") != 0) fclose(fp);
}

/*
 * Highlight/unhighligh spcified modifier key on the screen.
 */
static void Highlight(char *name, Boolean state)
{
  char name1[50];
  Widget w;

  sprintf(name1, "*%s", name);
  w = XtNameToWidget(toplevel, name1);
  if (w != None) {
    if (strstr(name, "Focus") != NULL) {
      if (target_dpy == dpy)
        XtVaSetValues(w, XtNbackground, appres.focus_background, NULL);
      else
        XtVaSetValues(w, XtNbackground, appres.remote_focus_background, NULL);
      if (state)
        XtVaSetValues(w, XtNforeground, appres.highlight_foreground, NULL);
      else
        XtVaSetValues(w, XtNforeground, appres.special_foreground, NULL);
    } else {
      if (state)
        XtVaSetValues(w, XtNbackground, appres.highlight_background,
                      XtNforeground, appres.highlight_foreground, NULL);
      else
        XtVaSetValues(w, XtNbackground, appres.special_background,
                      XtNforeground, appres.special_foreground, NULL);
    }
  }
}

/*
 * Highlight/unhighligh keys on the screen to reflect the state.
 */
static void RefreshShiftState(Boolean force)
{
  static Boolean first = TRUE;
  static int last_shift_state = 0;
  static int last_mouse_shift = 0;
  static int last_num_lock_state = FALSE;
  static Display *last_target_dpy = NULL;
  static long last_focus = 0;
  int cur_shift;
  int changed;
  int first_row, row, col;
  Boolean shifted;
  char *label;
  int mask;

  cur_shift = shift_state | mouse_shift;
  changed = cur_shift ^ (last_shift_state | last_mouse_shift);
  if (first || force) changed = 0xffff;

  if (changed & ShiftMask) {
    Highlight("Shift_L", cur_shift & ShiftMask);
    Highlight("Shift_R", cur_shift & ShiftMask);
  }
  if (changed & ControlMask) {
    Highlight("Control_L", cur_shift & ControlMask);
    Highlight("Control_R", cur_shift & ControlMask);
  }
  if (changed & alt_mask) {
    Highlight("Alt_L", cur_shift & alt_mask);
    Highlight("Alt_R", cur_shift & alt_mask);
  }
  if (changed & meta_mask) {
    Highlight("Meta_L", cur_shift & meta_mask);
    Highlight("Meta_R", cur_shift & meta_mask);
  }
  if (changed & LockMask) {
    Highlight("Caps_Lock", cur_shift & LockMask);
  }
  if (changed & altgr_mask) {
    Highlight("Mode_switch", cur_shift & altgr_mask);
  }
  if (last_num_lock_state != appres.num_lock_state) {
    Highlight("Num_Lock", appres.num_lock_state);
    Highlight("keypad_panel*Num_Lock", appres.num_lock_state);
  }
  if (last_target_dpy != target_dpy || last_focus != focused_window) {
    Highlight("Focus", focused_window != 0);
    Highlight("keypad*Focus", focused_window != 0);
    Highlight("keypad_panel*Focus", focused_window != 0);
    last_target_dpy = target_dpy;
    last_focus = focused_window;
  }

  mask = ShiftMask | LockMask | altgr_mask;
  changed = (shift_state & mask) ^ (last_shift_state & mask);
  if (first || force) changed = TRUE;
  if (changed && !appres.keypad_only
      && (appres.modal_keytop || toplevel_height < appres.modal_threshold)) {
    first_row = appres.function_key ? 0 : 1;
    for (row = first_row; row < NUM_KEY_ROWS; row++) {
      for (col = 0; col < NUM_KEY_COLS; col++) {
	shifted = (shift_state & ShiftMask);
	if (key_widgets[row][col] != None) {
	  if ((shift_state & altgr_mask) && altgr_key_labels[row][col] != NULL) {
	    if (shifted && shift_altgr_key_labels[row][col] != NULL)
	      label = shift_altgr_key_labels[row][col];
	    else
	      label = altgr_key_labels[row][col];
	  } else {
	    if ((shift_state & LockMask)
		&& isalpha(keys_normal[row][col][0]) && keys_normal[row][col][1] == '\0')
	      shifted = !shifted;
	    if (shifted && shift_key_labels[row][col] != NULL)
	      label = shift_key_labels[row][col];
	    else
	      label = normal_key_labels[row][col];
	  }
	  if (label == NULL) {
	    fprintf(stderr, "%s: no label for key %d,%d\n", PROGRAM_NAME, row, col);
	    label = "";
	  }
	  if (strcmp(label, "space") == 0) label = "";
	  XtVaSetValues(key_widgets[row][col], XtNlabel, label, NULL);
	}
      }
    }
  }

  last_shift_state = shift_state;
  last_mouse_shift = mouse_shift;
  last_num_lock_state = appres.num_lock_state;
  first = FALSE;

#ifdef USE_XTEST
  if (appres.xtest && strlen(appres.positive_modifiers) != 0) {
    /* modifiers specified in positiveModifiers resouce will be hold down
       so that it can be used with, for example, mouse operations */

    Window root, child;
    int root_x, root_y, x, y;
    unsigned int mask;

    XKeyEvent event;

    event.display = target_dpy;
    event.window = RootWindow(event.display, DefaultScreen(event.display));
    event.root = event.window;
    event.subwindow = None;
    event.time = CurrentTime;
    event.x = 1;
    event.y = 1;
    event.x_root = 1;
    event.y_root = 1;
    event.same_screen = TRUE;
    event.state = 0;

    XQueryPointer(target_dpy, event.root, &root, &child, &root_x, &root_y, &x, &y, &mask);

    if (strstr(appres.positive_modifiers, "shift") != NULL
	&& (shift_state & ShiftMask) != (mask & ShiftMask)) {
      event.keycode = XKeysymToKeycode(target_dpy, XK_Shift_L);
      event.type = (shift_state & ShiftMask) ? KeyPress : KeyRelease;
      SendEvent(&event);
    }
    if (strstr(appres.positive_modifiers, "control") != NULL
	&& (shift_state & ControlMask) != (mask & ControlMask)) {
      event.keycode = XKeysymToKeycode(target_dpy, XK_Control_L);
      event.type = (shift_state & ControlMask) ? KeyPress : KeyRelease;
      SendEvent(&event);
    }
    if (strstr(appres.positive_modifiers, "alt") != NULL
	&& (shift_state & alt_mask) != (mask & alt_mask)) {
      event.keycode = XKeysymToKeycode(target_dpy, XK_Alt_L);
      event.type = (shift_state & alt_mask) ? KeyPress : KeyRelease;
      SendEvent(&event);
    }
    if (strstr(appres.positive_modifiers, "meta") != NULL
	&& (shift_state & meta_mask) != (mask & meta_mask)) {
      event.keycode = XKeysymToKeycode(target_dpy, XK_Meta_L);
      event.type = (shift_state & meta_mask) ? KeyPress : KeyRelease;
      SendEvent(&event);
    }
  }
#endif
}


/*
 * This function will be called when mouse button is pressed on a key
 * on the screen.  Most operation will be performed in KeyPressed()
 * which will be called as callback for the Command widgets, and we
 * only need to check which mouse button is pressed here.
 */
static unsigned int n_key_repeat;

static void ButtonDownAction(Widget w, XEvent *event, String *pars, Cardinal *n_pars)
{
  n_key_repeat = 0;

  switch (event->xbutton.button) {
  case Button2:
    mouse_shift |= ControlMask;
    break;
  case Button3:
  case Button4:
    mouse_shift |= ShiftMask;
    break;
  }
  RefreshShiftState(FALSE);
}

/*
 * This function will be called when mouse button is released on a key
 * on the screen, after callback is called.
 */
static void ButtonUpAction(Widget w, XEvent *event, String *pars, Cardinal *n_pars)
{
  if (appres.quick_modifiers) {
    if (n_key_repeat == 1) XtCallCallbacks(w, XtNcallback, NULL);
  }
  mouse_shift = 0;
  RefreshShiftState(FALSE);
}

/*
 * Get the geometry of the base window.
 */
static char *GetWindowGeometry(Widget w)
{
  static char geom[50];

  Position x0, y0;
  Window root;
  int x1, y1;
  unsigned int wd, ht, bd, dp;

  XtVaGetValues(w, XtNx, &x0, XtNy, &y0, NULL);
  XGetGeometry(dpy, XtWindow(w), &root, &x1, &y1, &wd, &ht, &bd, &dp);
  sprintf(geom, "%dx%d+%d+%d", wd, ht, (int)(x0 - x1), (int)(y0 - y1));

  return geom;
}

/*
 * Set window manager hint.
 * ("Extended Window Manager Hints", http://standards.freedesktop.org/wm-spec/)
 */
static void SetWindowManagerHint(Boolean initial)
{
  if (initial) {
    if (appres.wm_toolbar) {
      Atom net_wm_window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
      Atom net_wm_window_type_toolbar = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
      XChangeProperty(dpy, XtWindow(toplevel),
	  net_wm_window_type, XA_ATOM, 32,
	  PropModeReplace,
	  (unsigned char *) &net_wm_window_type_toolbar, 1);
      if (appres.debug)
	fprintf(stderr, "SetWindowManagerHint: set _NET_WM_WINDOW_TYPE_TOOLBAR\n");
    }
  }

  if (!initial || appres.always_on_top) {
    const int net_wm_state_remove = 0;
    const int net_wm_state_add = 1;
    Atom net_wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
    Atom net_wm_state_above = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", False);
    XClientMessageEvent ev;
    ev.type = ClientMessage;
    ev.display = dpy;
    ev.window = XtWindow(toplevel);
    ev.message_type = net_wm_state;
    ev.format = 32;
    ev.data.l[0] = appres.always_on_top ? net_wm_state_add : net_wm_state_remove;
    ev.data.l[1] = net_wm_state_above;
    ev.data.l[2] = 0;
    XSendEvent(dpy, RootWindow(dpy, DefaultScreen(dpy)),
	       FALSE, SubstructureNotifyMask | SubstructureRedirectMask,
	       (XEvent *)&ev);
    if (appres.debug)
      fprintf(stderr, "SetWindowManagerHint: _NET_WM_STATE_ABOVE = %d\n", ev.data.l[0]);
  }
}

/*
 * Restart the program to (possibly) change the keyboard layout,
 * by loading the app-default file for the selected "customization".
 */
static void LayoutSelected(Widget w, char *key, char *data)
{
  static char *env_lang, *env_xenv;
  char name[50];
  char customization[30] = "", lang[30] = "C";
  char *xenv = NULL;

  int i;

  if (key != NULL) {
    if (strcmp(key, "default") != 0) {
      sscanf(key, "%29[^/]/%29s", customization, lang);
      sprintf(name, "XVkbd-%s", customization);
      xenv = XtResolvePathname(dpy, "app-defaults", name, NULL, NULL, NULL, 0, NULL);
      if (xenv == NULL) {
	fprintf(stderr, "%s: app-default file \"%s\" not installed\n",
		PROGRAM_NAME, name);
      }
    }

    env_lang = malloc(strlen("LC_ALL=") + strlen(lang) + 1);
    sprintf(env_lang, "LC_ALL=%s", lang);
    putenv(env_lang);
    if (xenv != NULL) {
      env_xenv = malloc(strlen("XENVIRONMENT=") + strlen(xenv) + 1);
      sprintf(env_xenv, "XENVIRONMENT=%s", xenv);
      putenv(env_xenv);
    } else if (getenv("XENVIRONMENT") != NULL) {
      putenv("XENVIRONMENT=");
    }
  }

  for (i = 1; i < argc1; i++) {
    if (strncmp(argv1[i], "-geom", strlen("-geom")) == 0) {
      if (appres.inherit_geometry) {
	argv1[i + 1] = GetWindowGeometry(toplevel);
      } else if (i + 2 == argc1) {
	argv1[i] = NULL;
	argc1 = i;
      }
      break;
    }
  }
  if (i == argc1 && appres.inherit_geometry) {
    argv1[argc1++] = "-geometry";
    argv1[argc1++] = GetWindowGeometry(toplevel);
    argv1[argc1] = NULL;
  }

  if (appres.debug) {
    fprintf(stderr, "XENVIRONMENT=%s, LC_ALL=%s\n", (xenv != NULL) ? xenv : "", lang);
    fprintf(stderr, "Exec:");
    for (i = 0; i < argc1; i++) fprintf(stderr, " %s", argv1[i]);
    fprintf(stderr, "\n");
  }

  execvp(argv1[0], argv1);
}

/*
 * Popup a window to select the (possibly) keyboard layout.
 * The "XVkbd.customizations" resource will define the list,
 * such as "default,german,swissgerman,french,latin1,jisx6004/ja".
 * For example, "german" here will make this program to load
 * "XVkbd-german" app-default file.  Locale for each configuration
 * can be specified by putting the locale name after "/".
 */
static void PopupLayoutPanel(void)
{
  static Widget layout_panel = None;

  char *customizations;
  char *cp, *cp2;
  Widget box, button;

  if (layout_panel == None) {
    layout_panel = XtVaCreatePopupShell("layout_panel", transientShellWidgetClass,
					toplevel, NULL);
    box = XtVaCreateManagedWidget("box", boxWidgetClass, layout_panel, NULL);

    customizations = XtNewString(appres.customizations);
    cp = strtok(customizations, " \t,");
    while (cp != NULL) {
      cp2 = strchr(cp, '/');
      if (cp2 != NULL) *cp2 = '\0';  /* temporary remove '/' */
      button = XtVaCreateManagedWidget(cp, commandWidgetClass, box, NULL);
      if (cp2 != NULL) *cp2 = '/';
      XtAddCallback(button, XtNcallback, (XtCallbackProc)LayoutSelected, XtNewString(cp));
      cp = strtok(NULL, " \t,");
    }
    XtRealizeWidget(layout_panel);
    XSetWMProtocols(dpy, XtWindow(layout_panel), &wm_delete_window, 1);

    XtFree(customizations);
  }

  XtPopup(layout_panel, XtGrabNone);
}

/*
 * Property panel
 */
static void SaveFunctionKey(void);  /* forward */

static Widget props_panel = None;
static Widget autoclick_buttons = None;
static Widget click_buttons = None;
static Boolean props_panel_active = FALSE;

static void PropsItemToggled(Widget w, char *key, char *data)
{
  Boolean last_wm_toolbar = appres.wm_toolbar;

  if (!props_panel_active) return;

#ifdef USE_XTEST
  XtVaGetValues(XtNameToWidget(props_panel, "*use_xtest"),
		XtNstate, &appres.xtest, NULL);
#endif
  XtVaGetValues(XtNameToWidget(props_panel, "*quick_modifiers"),
		XtNstate, &appres.quick_modifiers, NULL);
  XtVaGetValues(XtNameToWidget(props_panel, "*shift_lock"),
		XtNstate, &appres.shift_lock, NULL);
  XtVaGetValues(XtNameToWidget(props_panel, "*altgr_lock"),
		XtNstate, &appres.altgr_lock, NULL);
  XtVaGetValues(XtNameToWidget(props_panel, "*modifiers_lock"),
		XtNstate, &appres.modifiers_lock, NULL);
  XtVaGetValues(XtNameToWidget(props_panel, "*always_on_top"),
		XtNstate, &appres.always_on_top, NULL);
  XtVaGetValues(XtNameToWidget(props_panel, "*wm_toolbar"),
		XtNstate, &appres.wm_toolbar, NULL);
  XtVaGetValues(XtNameToWidget(props_panel, "*jump_pointer"),
		XtNstate, &appres.jump_pointer, NULL);

  appres.key_click_duration = (int)XawToggleGetCurrent(click_buttons);
  appres.autoclick_delay = (int)XawToggleGetCurrent(autoclick_buttons);

  SaveFunctionKey();
  SetWindowManagerHint(FALSE);

  if (appres.wm_toolbar != last_wm_toolbar) LayoutSelected(None, NULL, NULL);
}

static void PropsSetState(void)
{
#ifdef USE_XTEST
  XtVaSetValues(XtNameToWidget(props_panel, "*use_xtest"),
		XtNstate, appres.xtest, NULL);
#endif
  XtVaSetValues(XtNameToWidget(props_panel, "*quick_modifiers"),
		XtNstate, appres.quick_modifiers, NULL);
  XtVaSetValues(XtNameToWidget(props_panel, "*shift_lock"),
		XtNstate, appres.shift_lock, NULL);
  if (XtNameToWidget(toplevel, "*Mode_switch") == None) {
    XtSetSensitive(XtNameToWidget(props_panel, "*altgr_lock"), FALSE);
    XtVaSetValues(XtNameToWidget(props_panel, "*altgr_lock"),
		  XtNstate, FALSE, NULL);
  } else {
    XtVaSetValues(XtNameToWidget(props_panel, "*altgr_lock"),
		  XtNstate, appres.altgr_lock, NULL);
  }
  XtVaSetValues(XtNameToWidget(props_panel, "*modifiers_lock"),
		XtNstate, appres.modifiers_lock, NULL);
  XtVaSetValues(XtNameToWidget(props_panel, "*always_on_top"),
		XtNstate, appres.always_on_top, NULL);
  XtVaSetValues(XtNameToWidget(props_panel, "*wm_toolbar"),
		XtNstate, appres.wm_toolbar, NULL);

  XtVaSetValues(XtNameToWidget(props_panel, "*jump_pointer"),
		XtNstate, appres.jump_pointer, NULL);

  XawToggleSetCurrent(click_buttons, (XtPointer)appres.key_click_duration);
  XawToggleSetCurrent(autoclick_buttons, (XtPointer)appres.autoclick_delay);
}

static void ClosePropsPanel(void)
{
  XtPopdown(props_panel);
  XFlush(dpy);

  SaveFunctionKey();
  if (completion_panel != None) XtPopdown(completion_panel);
}

static void PopupPropsPanel(void)
{
  static char *props_items[] = {
    "quick_modifiers",
    "shift_lock", "altgr_lock", "modifiers_lock",
    "always_on_top",
    "wm_toolbar",
#ifdef USE_XTEST
    "use_xtest",
#endif
    "jump_pointer",
  };

  if (props_panel == None) {
    Widget label, button;
    Widget form, w;
    int i;
    int val;

    props_panel = XtVaCreatePopupShell("props_panel", transientShellWidgetClass,
				       toplevel, NULL);
    form = XtVaCreateManagedWidget("form", formWidgetClass, props_panel, NULL);

    w = None;
    for (i = 0; i < XtNumber(props_items); i++) {
      w = XtVaCreateManagedWidget(props_items[i], toggleWidgetClass,
				  form, XtNfromVert, w, NULL);
      XtAddCallback(w, XtNcallback, (XtCallbackProc)PropsItemToggled,
		    (XtPointer)props_items[i]);
    }

    label = XtVaCreateManagedWidget("click", labelWidgetClass,
				    form, XtNfromVert, w, NULL);
    button = XtVaCreateManagedWidget("OFF", toggleWidgetClass,
				     form, XtNfromVert, w, XtNfromHoriz, label,
				     XtNwidth, 0, XtNhorizDistance, 0, NULL);
    XtVaSetValues(button, XtNradioGroup, button, XtNradioData, (XtPointer)0, NULL);
    XtAddCallback(button, XtNcallback, (XtCallbackProc)PropsItemToggled,
		  (XtPointer)0);
    click_buttons = button;
    for (val = 1; val <= 50; val *= 2) {
      char s1[10];
      sprintf(s1, "%dms", val);
      button = XtVaCreateManagedWidget(s1, toggleWidgetClass,
			       form, XtNfromVert, w, XtNfromHoriz, button,
			       XtNradioData, (XtPointer)val,
			       XtNradioGroup, click_buttons,
			       XtNwidth, 0, XtNhorizDistance, 0, NULL);
      XtAddCallback(button, XtNcallback, (XtCallbackProc)PropsItemToggled,
		    NULL);
    }

    w = label;
    label = XtVaCreateManagedWidget("autoclick", labelWidgetClass,
				    form, XtNfromVert, w, NULL);
    button = XtVaCreateManagedWidget("OFF", toggleWidgetClass,
				     form, XtNfromVert, w, XtNfromHoriz, label,
				     XtNwidth, 0, XtNhorizDistance, 0, NULL);
    XtVaSetValues(button, XtNradioGroup, button, XtNradioData, (XtPointer)0, NULL);
    XtAddCallback(button, XtNcallback, (XtCallbackProc)PropsItemToggled,
		  (XtPointer)0);
    autoclick_buttons = button;
    for (val = 500; val <= 1000; val += 100) {
      char s1[10];
      sprintf(s1, "%dms", val);
      button = XtVaCreateManagedWidget(s1, toggleWidgetClass,
			       form, XtNfromVert, w, XtNfromHoriz, button,
			       XtNradioData, (XtPointer)val,
			       XtNradioGroup, autoclick_buttons,
			       XtNwidth, 0, XtNhorizDistance, 0, NULL);
      XtAddCallback(button, XtNcallback, (XtCallbackProc)PropsItemToggled,
		    (XtPointer)val);
    }

    w = label;
    label = XtVaCreateManagedWidget("dict_entry_label", labelWidgetClass,
				    form, XtNfromVert, w, NULL);
    props_dict_entry = XtVaCreateManagedWidget("dict_entry", asciiTextWidgetClass, form,
					  XtNfromVert, w, XtNfromHoriz, label,
					  XtNuseStringInPlace, True,
					  XtNstring, dict_filename,
					  XtNeditType, XawtextEdit,
					  XtNlength, sizeof(dict_filename) - 1,
					  NULL);
    button = XtVaCreateManagedWidget("dict_default_button", commandWidgetClass,
			     form, XtNfromVert, w, XtNfromHoriz, props_dict_entry,
			     NULL);
    XtAddCallback(button, XtNcallback, (XtCallbackProc)SetDefaultDictionary, NULL);
    

    w = XtVaCreateManagedWidget("dismiss", commandWidgetClass, form,
				XtNfromVert, label, NULL);
    XtAddCallback(w, XtNcallback, (XtCallbackProc)ClosePropsPanel, NULL);

    XtRealizeWidget(props_panel);
    XSetWMProtocols(dpy, XtWindow(props_panel), &wm_delete_window, 1);
  }
  XtPopup(props_panel, XtGrabNone);
  PropsSetState();

  props_panel_active = TRUE;

  if (completion_panel != None) XtPopdown(completion_panel);
}

/*
 * Callback for main menu (activated from "xvkbd" logo).
 */
static Widget about_panel = None;
static Widget keypad_panel = None;
static Widget sun_fkey_panel = None;
static Widget deadkey_panel = None;
static Widget display_panel = None;
static Widget display_status = None;

#define DISPLAY_NAME_LENGTH 50

static void OpenRemoteDisplay(Widget w, char *display_name, char *data)
{
  static char name[DISPLAY_NAME_LENGTH + 10];
  char *cp;

  focused_window = None;
  focused_subwindow = None;
  if (target_dpy != NULL && target_dpy != dpy) XCloseDisplay(target_dpy);

  strncpy(name, (display_name == NULL) ? "" : display_name, sizeof(name));
  for (cp = name; isascii(*cp) && isprint(*cp); cp++) ;
  *cp = '\0';

  if (strlen(name) == 0) {
    target_dpy = dpy;
    XtVaSetValues(display_status, XtNlabel, "Disconnected - local display selected", NULL);
    XtPopdown(display_panel);
  } else {
    if (strchr(name, ':') == NULL) strcat(name, ":0");
    target_dpy = XOpenDisplay(name);
    if (target_dpy == NULL) {
      XtVaSetValues(display_status, XtNlabel, "Couldn't connect to the display", NULL);
      target_dpy = dpy;
      XBell(dpy, 0);
    } else {
      XtVaSetValues(display_status, XtNlabel, "Connected", NULL);
      XtPopdown(display_panel);
    }
  }

  ReadKeymap();
  if (!altgr_mask && appres.auto_add_keysym) AddModifier(XK_Mode_switch);

  RefreshMainMenu();
  RefreshShiftState(FALSE);
}

static void MenuSelected(Widget w, char *key)
{
  Widget form;
  
  if (strcmp(key, "man") == 0) {
    if (!appres.secure) system(appres.show_manual_command);
  } else if (strcmp(key, "about") == 0) {
    if (about_panel == None) {
      about_panel = XtVaCreatePopupShell("about_panel", transientShellWidgetClass,
					  toplevel, NULL);
      XtVaCreateManagedWidget("message", labelWidgetClass, about_panel,
			      XtNlabel, appres.description, NULL);
      XtRealizeWidget(about_panel);
      XSetWMProtocols(dpy, XtWindow(about_panel), &wm_delete_window, 1);
    }
    XtPopup(about_panel, XtGrabNone);
  } else if (strcmp(key, "keypad") == 0) {
    if (keypad_panel == None) {
      keypad_panel = XtVaCreatePopupShell("keypad_panel", transientShellWidgetClass,
					  toplevel, NULL);
      form = XtVaCreateManagedWidget("form", formWidgetClass, keypad_panel, NULL);
      MakeKeypad(form, None, None);
      XtRealizeWidget(keypad_panel);
      XSetWMProtocols(dpy, XtWindow(keypad_panel), &wm_delete_window, 1);
    }
    XtPopup(keypad_panel, XtGrabNone);
  } else if (strcmp(key, "sun_fkey") == 0) {
    if (sun_fkey_panel == None) {
      sun_fkey_panel = XtVaCreatePopupShell("sun_fkey_panel", transientShellWidgetClass,
					  toplevel, NULL);
      form = XtVaCreateManagedWidget("form", formWidgetClass, sun_fkey_panel, NULL);
      MakeSunFunctionKey(form, None, None);
      XtRealizeWidget(sun_fkey_panel);
      XSetWMProtocols(dpy, XtWindow(sun_fkey_panel), &wm_delete_window, 1);
    }
    XtPopup(sun_fkey_panel, XtGrabNone);
  } else if (strcmp(key, "deadkey") == 0) {
    if (deadkey_panel == None) {
      deadkey_panel = XtVaCreatePopupShell("deadkey_panel", transientShellWidgetClass,
					  toplevel, NULL);
      form = XtVaCreateManagedWidget("form", formWidgetClass, deadkey_panel, NULL);
      MakeDeadkeyPanel(form);
      XtRealizeWidget(deadkey_panel);
      XSetWMProtocols(dpy, XtWindow(deadkey_panel), &wm_delete_window, 1);
    }
    XtPopup(deadkey_panel, XtGrabNone);
  } else if (strcmp(key, "completion") == 0) {
    PopupCompletionPanel();
  } else if (strcmp(key, "select_layout") == 0) {
    PopupLayoutPanel();
  } else if (strcmp(key, "edit_fkey") == 0) {
    PopupFunctionKeyEditor();
  } else if (strcmp(key, "show_keypad") == 0
	     || strcmp(key, "show_functionkey") == 0) {
    if (strcmp(key, "show_keypad") == 0) appres.keypad = !appres.keypad;
    else appres.function_key = !appres.function_key;
    MakeKeyboard(TRUE);
  } else if (strcmp(key, "props") == 0) {
    PopupPropsPanel();
  } else if (strcmp(key, "open_display") == 0) {
    if (display_panel == None) {
      Widget label, entry, button;
      static char display_name[DISPLAY_NAME_LENGTH] = "";
      display_panel = XtVaCreatePopupShell("display_panel", transientShellWidgetClass,
					  toplevel, NULL);
      form = XtVaCreateManagedWidget("form", formWidgetClass, display_panel, NULL);
      label = XtVaCreateManagedWidget("label", labelWidgetClass, form, NULL);
      entry = XtVaCreateManagedWidget("entry", asciiTextWidgetClass, form,
				      XtNfromHoriz, label,
				      XtNuseStringInPlace, True,
				      XtNeditType, XawtextEdit,
				      XtNstring, display_name,
				      XtNlength, sizeof(display_name) - 1,
				      NULL);

      button = XtVaCreateManagedWidget("ok", commandWidgetClass, form,
				       XtNfromHoriz, entry, NULL);
      XtAddCallback(button, XtNcallback, (XtCallbackProc)OpenRemoteDisplay, (XtPointer)display_name);

      display_status = XtVaCreateManagedWidget("status", labelWidgetClass, form,
					       XtNfromVert, label,
					       XtNlabel, "", NULL);
      XtRealizeWidget(display_panel);
      XSetWMProtocols(dpy, XtWindow(display_panel), &wm_delete_window, 1);

      XtSetKeyboardFocus(display_panel, entry);
    }
    XtPopup(display_panel, XtGrabNone);
  } else if (strcmp(key, "close_display") == 0) {
    OpenRemoteDisplay(None, NULL, NULL);
  } else if (strcmp(key, "quit") == 0) {
    DeleteWindowProc(None, NULL, NULL, NULL);
  }
}

static void ClosePopupPanel(Widget w)
{
  if (w == keypad_panel) {
    XtDestroyWidget(w);
    keypad_panel = None;
  } else if (w == props_panel) {
    ClosePropsPanel();
  } else {
    XtPopdown(w);
  }
}

/*
 * Iconify/uniconify the xvkbd window even if window manager is not
 * available.
 */
static void IconifyWindow(Widget w, Boolean iconify)
{
  static Widget iconified_window = None;
  static Widget uniconify_button = None;
  static Position x0, y0;
  static int x1, y1;
  static unsigned int wd, ht, bd, dp;

  if (iconify) {
    Window root;
    int i;

    XUnmapWindow(dpy, XtWindow(toplevel));

    if (iconified_window == None) {
      Widget box;

      iconified_window = XtVaCreatePopupShell("iconified_window", transientShellWidgetClass,
					      toplevel, XtNoverrideRedirect, TRUE, NULL);
      box = XtVaCreateManagedWidget("form", boxWidgetClass, iconified_window, NULL);
      uniconify_button = XtVaCreateManagedWidget("uniconify_button", commandWidgetClass, box,
						 XtNbitmap, xvkbd_pixmap,
						 XtNhorizDistance, 10, XtNvertDistance, 0,
						 NULL);
      XtAddCallback(uniconify_button, XtNcallback, (XtCallbackProc)IconifyWindow, FALSE);

      XtRealizeWidget(iconified_window);
      XSetWMProtocols(dpy, XtWindow(iconified_window), &wm_delete_window, 1);
    }

    XtVaGetValues(toplevel, XtNx, &x0, XtNy, &y0, NULL);
    XGetGeometry(dpy, XtWindow(toplevel), &root, &x1, &y1, &wd, &ht, &bd, &dp);

    XMoveResizeWindow(dpy, XtWindow(iconified_window), x0 + bd, y0 + bd, wd, ht);
    XtPopup(iconified_window, XtGrabNone);
    for (i = 9; 0 < i; i--) {
      Dimension btn_wd, btn_ht;
      Dimension wd1, ht1;

      wd1 = wd * i / 10;
      ht1 = ht * i / 10;
      XtVaGetValues(uniconify_button, XtNwidth, &btn_wd, XtNheight, &btn_ht, NULL);
      if (i == 1 || wd1 < btn_wd) wd1 = btn_wd;
      if (i == 1 || ht1 < btn_ht) ht1 = btn_ht;
      XMoveResizeWindow(dpy, XtWindow(iconified_window), x0 + bd, y0 + (ht - ht1) + bd, wd1, ht1);
      XFlush(dpy);
      usleep(10000);
    }
  } else {
    if (iconified_window != None) XtPopdown(iconified_window);
    XMapWindow(dpy, XtWindow(toplevel));
  }
}

static void SignalUser1(void)
{
  XWindowAttributes attr;
  XGetWindowAttributes(dpy, XtWindow(toplevel), &attr);
  IconifyWindow(None, attr.map_state != IsUnmapped);
  XSync(dpy, FALSE);
}

/*
 * This will be called when user pressed a key on the screen.
 */
static const char *FindFunctionKeyValue(const char *key, Boolean shiftable);
static void ShowBalloon(Widget w, XEvent *event, String *pars, Cardinal *n_pars);
static void KeyClick(void);
static void StopAutoclick(void);

static void KeyPressed(Widget w, char *key, char *data)
{
  int row, col;
  int cur_shift;
  char *key1;
  KeySym keysym;
  Boolean shifted;
  const char *value;
  Boolean found;

  if (appres.debug) fprintf(stderr, "KeyPressed: key=%s, widget=%lx\n", key, (long)w);

  value = FindFunctionKeyValue(key, TRUE);
  if (value != NULL) {
    if (appres.debug) fprintf(stderr, "Assigned string: %s\n", value);
    if (value[0] == '!') {
      if (appres.debug) fprintf(stderr, "Launching: %s\n", value + 1);
      if (!appres.secure) system(value + 1);
    } else {
      if (value[0] == '\\') value = value + 1;
      if (appres.debug) fprintf(stderr, "Sending: %s\n", value);
      SendString(value);
    }
    ShowBalloon(w, NULL, NULL, NULL);
    return;
  }

  if (strncmp(key, "Shift", strlen("Shift")) == 0) {
    if (shift_state & ShiftMask) SendKeyPressedEvent(NoSymbol, shift_state);
    shift_state ^= ShiftMask;
  } else if (strncmp(key, "Control", strlen("Control")) == 0) {
    if (shift_state & ControlMask) SendKeyPressedEvent(NoSymbol, shift_state);
    shift_state ^= ControlMask;
  } else if (strncmp(key, "Alt", strlen("Alt")) == 0) {
    if (shift_state & alt_mask) SendKeyPressedEvent(NoSymbol, shift_state);
    shift_state ^= alt_mask;
  } else if (strncmp(key, "Meta", strlen("Meta")) == 0) {
    if (shift_state & meta_mask) SendKeyPressedEvent(NoSymbol, shift_state);
    shift_state ^= meta_mask;
  } else if (strcmp(key, "Caps_Lock") == 0) {
    if (shift_state & LockMask) SendKeyPressedEvent(NoSymbol, shift_state);
    shift_state ^= LockMask;
  } else if (strcmp(key, "Mode_switch") == 0) {
    if (shift_state & altgr_mask) SendKeyPressedEvent(NoSymbol, shift_state);
    shift_state ^= altgr_mask;
  } else if (strcmp(key, "Num_Lock") == 0) {
    appres.num_lock_state = !appres.num_lock_state;
  } else if (strcmp(key, "Focus") == 0) {
    cur_shift = shift_state | mouse_shift;
    if (cur_shift & ShiftMask) {
      focused_window = None;
      focused_subwindow = None;
    } else {
      GetFocusedWindow();
    }
  } else {
    if (appres.quick_modifiers && mouse_shift == 0 && w != None) {
      Window junk_w;
      int junk_i;
      unsigned junk_u;
      int cur_x, cur_y;
      Dimension btn_wd, btn_ht;

      n_key_repeat = n_key_repeat + 1;
      if (n_key_repeat == 1) return;

      XtVaGetValues(w, XtNwidth, &btn_wd, XtNheight, &btn_ht, NULL);
      XQueryPointer(dpy, XtWindow(w), &junk_w, &junk_w,
		    &junk_i, &junk_i, &cur_x, &cur_y, &junk_u);

      mouse_shift = 0;
      if (cur_x < 0 && btn_ht < cur_y) {
	mouse_shift |= alt_mask;  /* left-down */
      } else {
	if (cur_y < 0) mouse_shift |= ShiftMask;  /* up */
	else if (btn_ht < cur_y) mouse_shift |= meta_mask;  /* down */
	if (cur_x < 0) mouse_shift |= ControlMask;  /* left */
	else if (btn_wd < cur_x) mouse_shift |= altgr_mask;  /* right */
      }
    }
    cur_shift = shift_state | mouse_shift;
    shifted = (cur_shift & ShiftMask);
    key1 = key;
    if (w != None) {
      if (sscanf(key, "pad%d,%d", &row, &col) == 2) {
	if (appres.num_lock_state) shifted = !shifted;
	key1 = shifted ? keypad_shift[row][col]: keypad[row][col];
      } else {
	found = FALSE;
	if (sscanf(key, "%d,%d", &row, &col) == 2) {
	  found = TRUE;
	} else if (w != None) {
	  int first_row = appres.function_key ? 0 : 1;
	  for (row = first_row; row < NUM_KEY_ROWS; row++) {
	    for (col = 0; col < NUM_KEY_COLS; col++) {
	      if (key_widgets[row][col] == w) {
		found = TRUE;
		break;
	      }
	    }
	    if (col < NUM_KEY_COLS) break;
	  }
	}
	if (found) {
	  if ((cur_shift & LockMask)
	      && isalpha(keys_normal[row][col][0]) && keys_normal[row][col][1] == '\0')
	    shifted = !shifted;
	  if ((cur_shift & altgr_mask) && keys_altgr[row][col] != NULL) {
	    if (shifted && keys_shift_altgr[row][col] != NULL) {
	      key1 = keys_shift_altgr[row][col];
	      if (strcmp(keys_altgr[row][col], keys_shift_altgr[row][col]) != 0)
		cur_shift &= ~ShiftMask;
	    } else {
	      key1 = keys_altgr[row][col];
	    }
	  } else {
	    if (shifted && keys_shift[row][col] != NULL) {
	      key1 = keys_shift[row][col];
	      if (strcmp(keys_normal[row][col], keys_shift[row][col]) != 0)
		cur_shift &= ~ShiftMask;
	    } else {
	      key1 = keys_normal[row][col];
	    }
	  }
	}  /* if (found) ... */
      }  /* if (sscanf(key, "pad%d,%d", ... */
    }  /* if (w != None) ... */
    if (strlen(key1) == 1) {
      SendKeyPressedEvent((KeySym)*key1 & 0xff, cur_shift);
      AddToCompletionText((KeySym)*key1);
    } else {
      if (islower(key1[0]) && key1[1] == ':') {
	switch (key1[0]) {
	case 's': cur_shift |= ShiftMask; break;
	case 'c': cur_shift |= ControlMask; break;
	case 'a': cur_shift |= alt_mask; break;
	case 'm': cur_shift |= meta_mask; break;
	default: fprintf(stderr, "%s: unknown modidier: %s\n",
			 PROGRAM_NAME, key1); break;
	}
	key1 = key1 + 2;
      }
      keysym = XStringToKeysym(key1);
      if ((!appres.keypad_keysym && strncmp(key1, "KP_", 3) == 0)
	  || XKeysymToKeycode(target_dpy, keysym) == NoSymbol) {
	switch ((unsigned)keysym) {
	case XK_KP_Equal: keysym = XK_equal; break;
	case XK_KP_Divide: keysym = XK_slash; break;
	case XK_KP_Multiply: keysym = XK_asterisk; break;
	case XK_KP_Add: keysym = XK_plus; break;
	case XK_KP_Subtract: keysym = XK_minus; break;
	case XK_KP_Enter: keysym = XK_Return; break;
	case XK_KP_1: keysym = XK_1; break;
	case XK_KP_2: keysym = XK_2; break;
	case XK_KP_3: keysym = XK_3; break;
	case XK_KP_4: keysym = XK_4; break;
	case XK_KP_5: keysym = XK_5; break;
	case XK_KP_6: keysym = XK_6; break;
	case XK_KP_7: keysym = XK_7; break;
	case XK_KP_8: keysym = XK_8; break;
	case XK_KP_9: keysym = XK_9; break;
	case XK_Shift_L: keysym = XK_Shift_R; break;
	case XK_Shift_R: keysym = XK_Shift_L; break;
	case XK_Control_L: keysym = XK_Control_R; break;
	case XK_Control_R: keysym = XK_Control_L; break;
	case XK_Alt_L: keysym = XK_Alt_R; break;
	case XK_Alt_R: keysym = XK_Alt_L; break;
	case XK_Meta_L: keysym = XK_Meta_R; break;
	case XK_Meta_R: keysym = XK_Meta_L; break;
	default:
	  if (keysym == NoSymbol || !appres.auto_add_keysym)
	    fprintf(stderr, "%s: no such key: %s\n",
		    PROGRAM_NAME, key1); break;
	}
      }
      SendKeyPressedEvent(keysym, cur_shift);
      AddToCompletionText(keysym);

      if ((cur_shift & ControlMask) && (cur_shift & alt_mask)) {
        if (strstr(XServerVendor(dpy), "XFree86") != NULL) {
          if (strcmp(key1, "KP_Add") == 0) {
            if (!appres.secure) system("xvidtune -next");
          } else if (strcmp(key1, "KP_Subtract") == 0) {
            if (!appres.secure) system("xvidtune -prev");
          }
        }
      }
    }
    if (!appres.shift_lock)
      shift_state &= ~ShiftMask;
    if (!appres.modifiers_lock)
      shift_state &= ~(ControlMask | alt_mask | meta_mask);
    if (!appres.altgr_lock)
      shift_state &= ~altgr_mask;
  }
  RefreshShiftState(FALSE);

  if (w != None) {
    KeyClick();
/*     StopAutoclick(); */
  }
}

/*
 * Redefine keyboard layout.
 * "spec" is a sequence of words separated with spaces, and it is
 * usally specified in app-defaults file, as:
 * 
 *   xvkbd.AltGrKeys: \
 *      F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 F11 F12 BackSpace \n\
 *      Escape \271 \262 \263 \243 \254 \251 { [ ] } \\ ' ^ ' \n\
 *      ...
 *
 * White spaces separate the keys, and " \n" (note that white space
 * before the \n) separate the rows of keys.
 */
static void RedefineKeys(char *array[NUM_KEY_ROWS][NUM_KEY_COLS], const char *spec)
{
  char *s = XtNewString(spec);
  char *cp;
  int row, col;
  int key_rows = NUM_KEY_ROWS;
  int key_cols = NUM_KEY_COLS;

  for (row = 0; row < key_rows; row++) {
    for (col = 0; col < key_cols; col++) array[row][col] = NULL;
  }
  row = 0;
  col = 0;
  cp = strtok(s, " ");
  while (cp != NULL) {
    if (*cp == '\n') {
      row = row + 1;
      col = 0;
      cp = cp + 1;
    }
    if (*cp != '\0') {
      if (key_rows <= row) {
        fprintf(stderr, "%s: too many key rows: \"%s\" ignored\n",
                PROGRAM_NAME, cp);
      } else if (key_cols <= col) {
        fprintf(stderr, "%s: too many keys in a row: \"%s\" ignored\n",
                PROGRAM_NAME, cp);
      } else {
        array[row][col] = XtNewString(cp);
        col = col + 1;
      }
    }
    cp = strtok(NULL, " ");
  }
  XtFree(s);
}

/*
 * Create keyboard on the screen.
 */
static Widget MakeKey(Widget parent, const char *name, const char *label, Pixel color)
{
  static Pixmap up_pixmap = None;
  static Pixmap down_pixmap = None;
  static Pixmap left_pixmap = None;
  static Pixmap right_pixmap = None;
  static Pixmap back_pixmap = None;
  Widget w;
  Window scr = RootWindow(dpy, DefaultScreen(dpy));
  char str[50];
  int len;

  if (!appres.auto_repeat
      || strncmp(name, "Shift", strlen("Shift")) == 0
      || strncmp(name, "Control", strlen("Control")) == 0
      || strncmp(name, "Alt", strlen("Alt")) == 0
      || strncmp(name, "Meta", strlen("Meta")) == 0
      || strcmp(name, "Caps_Lock") == 0
      || strcmp(name, "Mode_switch") == 0
      || strcmp(name, "Num_Lock") == 0
      || strcmp(name, "Focus") == 0) {
    w = XtVaCreateManagedWidget(name, commandWidgetClass, parent,
                                XtNbackground, color, NULL);
  } else {
    w = XtVaCreateManagedWidget(name, repeaterWidgetClass, parent,
                                XtNbackground, color, NULL);
  }
  XtAddCallback(w, XtNcallback, (XtCallbackProc)KeyPressed, (XtPointer)name);

  if (label != NULL) {
    strncpy(str, label, sizeof(str) - 1);
    if (strcmp(str, "space") == 0) strcpy(str, "");
    len = strlen(str);
    if (3 <= len) {
      if (str[1] == '_') str[1] = ' ';
      if (str[len - 2] == '_') str[len - 2] = ' ';
    }
    XtVaSetValues(w, XtNlabel, str, NULL);

    if (strcmp(label, "up") == 0) {
      if (up_pixmap == None)
        up_pixmap = XCreateBitmapFromData(dpy, scr,
                                (char *)up_bits, up_width, up_height);
      XtVaSetValues(w, XtNbitmap, up_pixmap, NULL);
    } else if (strcmp(label, "down") == 0) {
      if (down_pixmap == None)
        down_pixmap = XCreateBitmapFromData(dpy, scr,
                                (char *)down_bits, down_width, down_height);
      XtVaSetValues(w, XtNbitmap, down_pixmap, NULL);
    } else if (strcmp(label, "left") == 0) {
      if (left_pixmap == None)
        left_pixmap = XCreateBitmapFromData(dpy, scr,
                                (char *)left_bits, left_width, left_height);
      XtVaSetValues(w, XtNbitmap, left_pixmap, NULL);
    } else if (strcmp(label, "right") == 0) {
      if (right_pixmap == None)
        right_pixmap = XCreateBitmapFromData(dpy, scr,
                                (char *)right_bits, right_width, right_height);
      XtVaSetValues(w, XtNbitmap, right_pixmap, NULL);
    } else if (strcmp(label, "back") == 0) {
      if (back_pixmap == None)
        back_pixmap = XCreateBitmapFromData(dpy, scr,
                                (char *)back_bits, back_width, back_height);
      XtVaSetValues(w, XtNbitmap, back_pixmap, NULL);
    }
  }

  return w;
}

static void MakeKeypad(Widget form, Widget from_vert, Widget from_horiz)
{
  Widget key, left, upper;
  Pixel color;
  XFontStruct *font;
  int row, col;
  Widget keypad_box;
  Widget keypad_row[NUM_KEYPAD_ROWS];
  char name[50];

  keypad_box = XtVaCreateManagedWidget("keypad", formWidgetClass, form, NULL);
  if (from_horiz != None)
    XtVaSetValues(keypad_box, XtNfromHoriz, from_horiz, NULL);
  else
    XtVaSetValues(keypad_box, XtNhorizDistance, 0, NULL);
  if (from_vert != None)
    XtVaSetValues(keypad_box, XtNfromVert, from_vert, NULL);
  else
    XtVaSetValues(keypad_box, XtNvertDistance, 0, NULL);
  upper = None;
  for (row = 0; row < NUM_KEYPAD_ROWS; row++) {
    left = None;
    for (col = 0; keypad[row][col] != NULL; col++) {
      font = appres.keypad_font;
      if (strlen(keypad_label[row][col]) == 1) font = appres.letter_font;
      color = appres.special_background;
      if (strcmp(keypad[row][col], "Focus") == 0)
	color = appres.focus_background;
      else if (strcmp(keypad_shift[row][col], ".") == 0
	       || (strncmp(keypad_shift[row][col], "KP_", 3) == 0
		   && isdigit(keypad_shift[row][col][3])))
	color = appres.general_background;
      strcpy(name, keypad[row][col]);
      if (strcmp(name, "Focus") != 0 && strcmp(name, "Num_Lock") != 0)
	sprintf(name, "pad%d,%d", row, col);
      key = MakeKey(keypad_box, XtNewString(name),
		    keypad_label[row][col], color);
      XtVaSetValues(key, XtNfont, font, NULL);
      if (row != 0)
	XtVaSetValues(key, XtNfromVert, keypad_row[row - 1], NULL);
      if (left != None)
	XtVaSetValues(key, XtNfromHoriz, left, NULL);
      if (col == 0)
	keypad_row[row] = key;
      left = key;
    }
  }
}

static void MakeSunFunctionKey(Widget form, Widget from_vert, Widget from_horiz)
{
  Widget key, left, upper;
  int row, col;
  Widget fkey_box;
  Widget fkey_row[NUM_SUN_FKEY_ROWS];

  fkey_box = XtVaCreateManagedWidget("fkey", formWidgetClass, form, NULL);
  if (from_horiz != None)
    XtVaSetValues(fkey_box, XtNfromHoriz, from_horiz, NULL);
  else
    XtVaSetValues(fkey_box, XtNhorizDistance, 0, NULL);
  if (from_vert != None)
    XtVaSetValues(fkey_box, XtNfromVert, from_vert, NULL);
  else
    XtVaSetValues(fkey_box, XtNvertDistance, 0, NULL);
  upper = None;
  for (row = 0; row < NUM_SUN_FKEY_ROWS; row++) {
    left = None;
    for (col = 0; sun_fkey[row][col] != NULL; col++) {
      key = MakeKey(fkey_box, sun_fkey[row][col],
		    sun_fkey_label[row][col], appres.special_background);
      XtVaSetValues(key, XtNfont, appres.keypad_font, NULL);
      if (row != 0)
	XtVaSetValues(key, XtNfromVert, fkey_row[row - 1], NULL);
      if (left != None)
	XtVaSetValues(key, XtNfromHoriz, left, NULL);
      if (col == 0)
	fkey_row[row] = key;
      left = key;
    }
  }
}

static void MakeDeadkeyPanel(Widget form)
{
  Widget deadkey_box, left, key;
  char *deadkeys, *cp, *cp2;

  deadkeys = XtNewString(appres.deadkeys);

  deadkey_box = XtVaCreateManagedWidget("deadkey", formWidgetClass, form, NULL);
  left = None;
  cp = strtok(deadkeys, " \t,");
  while (cp != NULL) {
    cp2 = XtNewString(cp);
    key = MakeKey(deadkey_box, cp2, NULL, appres.general_background);
    if (left != None) XtVaSetValues(key, XtNfromHoriz, left, NULL);
    left = key;
    cp = strtok(NULL, " \t,");
  }
  XtFree(deadkeys);
}

static void RefreshMainMenu(void)
{
  static Pixmap check_pixmap = None;

  if (check_pixmap == None) {
    check_pixmap = XCreateBitmapFromData(dpy, RootWindow(dpy, DefaultScreen(dpy)),
				 (char *)check_bits, check_width, check_height);
  }
  XtVaSetValues(XtNameToWidget(main_menu, "*show_keypad"),
		XtNrightBitmap, appres.keypad ? check_pixmap : None, NULL);
  XtVaSetValues(XtNameToWidget(main_menu, "*show_functionkey"),
		XtNrightBitmap, appres.function_key ? check_pixmap : None, NULL);

  XtSetSensitive(XtNameToWidget(main_menu, "*edit_fkey"), appres.function_key);
  XtSetSensitive(XtNameToWidget(main_menu, "*close_display"), target_dpy != dpy);
}

static void MakeKeyboard(Boolean remake)
{
  static char *main_menu_items[] = {
    "about", "man", "keypad", "sun_fkey", "deadkey", "completion", "",
    "select_layout",
    "edit_fkey",
    "show_keypad",
    "show_functionkey",
    "props",
    "",
    "open_display", "close_display", "",
    "quit" };

  Widget form, key, left;
  Pixel color;
  XFontStruct *font;
  Dimension wd, max_wd;
  int row, col, first_row;
  char name[50], *label;
  Widget key_box[NUM_KEY_ROWS];
  Widget menu_entry;
  int i;

#include "xvkbd.xbm"
#include "iconify.xbm"

  if (remake) {
    appres.geometry = GetWindowGeometry(toplevel);
    XtUnrealizeWidget(toplevel);
    XtDestroyWidget(XtNameToWidget(toplevel, "form"));
  }

  form = XtVaCreateManagedWidget("form", formWidgetClass, toplevel, NULL);

  key_box[0] = None;
  key_box[1] = None;
  first_row = appres.function_key ? 0 : 1;
  if (!appres.keypad_only) {
    for (row = first_row; row < NUM_KEY_ROWS; row++) {
      if (keys_normal[row][0] == NULL) continue;

      sprintf(name, "row%d", row);
      key_box[row] = XtVaCreateManagedWidget(name, formWidgetClass, form, NULL);
      key_box[row + 1] = None;
      if (row != first_row)
        XtVaSetValues(key_box[row], XtNfromVert, key_box[row - 1], NULL);
      else if (!appres.function_key)
        XtVaSetValues(key_box[row], XtNvertDistance, 0, NULL);
        
      left = None;
      for (col = 0; keys_normal[row][col] != NULL; col++) {
        strcpy(name, keys_normal[row][col]);
	if (strcmp(name, "MainMenu") == 0) {
	  Widget iconify_button = None;

	  if (appres.minimizable) {
	    Pixmap iconify_pixmap = XCreateBitmapFromData(dpy, RootWindow(dpy, DefaultScreen(dpy)),
						  (char *)iconify_bits, iconify_width, iconify_height);
	    iconify_button = XtVaCreateManagedWidget("Iconify", commandWidgetClass, key_box[row],
						     XtNbitmap, iconify_pixmap, NULL);
	    XtAddCallback(iconify_button, XtNcallback, (XtCallbackProc)IconifyWindow, (void *)TRUE);
	  }

	  xvkbd_pixmap = XCreateBitmapFromData(dpy, RootWindow(dpy, DefaultScreen(dpy)),
			       (char *)xvkbd_bits, xvkbd_width, xvkbd_height);
	  key = XtVaCreateManagedWidget("MainMenu", menuButtonWidgetClass, key_box[row],
					XtNbitmap, xvkbd_pixmap, XtNfromHoriz, iconify_button, NULL);
	  main_menu = XtVaCreatePopupShell("menu", simpleMenuWidgetClass, key, NULL);
	  for (i = 0; i < XtNumber(main_menu_items); i++) {
	    if (strlen(main_menu_items[i]) == 0) {
	      XtVaCreateManagedWidget("separator", smeLineObjectClass, main_menu, NULL);
	    } else {
	      menu_entry = XtVaCreateManagedWidget(main_menu_items[i], smeBSBObjectClass,
						   main_menu, NULL);
	      XtAddCallback(menu_entry, XtNcallback, (XtCallbackProc)MenuSelected,
			    (XtPointer)main_menu_items[i]);
	    }
	  }
	} else {
	  label = appres.modal_keytop ? normal_key_labels[row][col] : key_labels[row][col];
	  if (isascii(name[0]) && isupper(name[0])) {
	    if (strcmp(name, "Focus") == 0) {
	      color = appres.focus_background;
	      font = appres.keypad_font;
	    } else {
	      color = appres.special_background;
	      if (label != NULL && strchr(label, '\n') != NULL) font = appres.keypad_font;
	      else font = appres.special_font;
	    }
	  } else {
	    color = appres.general_background;
	    font = appres.general_font;
	    if (isalpha(name[0])) font = appres.letter_font;
	    if (strcmp(name, "space") != 0) sprintf(name, "%d,%d", row, col);
	  }
	  key = MakeKey(key_box[row], XtNewString(name), label, color);
	  XtVaGetValues(key, XtNwidth, &wd, NULL);
	  if (wd <= 1) {
	    /* keys can be removed by setting its width to 1 */
	    XtDestroyWidget(key);
	    key = None;
	  } else {
	    XtVaSetValues(key, XtNfont, font, NULL);
#ifdef USE_I18N
	    if (font == appres.special_font || font == appres.keypad_font)
	      XtVaSetValues(key, XtNfontSet, appres.special_fontset, NULL);
#endif
	  }
	}
	if (key != None) {
	  if (left != None) XtVaSetValues(key, XtNfromHoriz, left, NULL);
	  left = key;
	}
	key_widgets[row][col] = key;
      }
    }
  }

   if (appres.keypad) MakeKeypad(form, key_box[0], key_box[1]);

  if (!appres.keypad_only && appres.function_key && appres.keypad) {
    XtVaCreateManagedWidget("banner", labelWidgetClass, form,
                            XtNfromHoriz, key_box[1],
                            XtNlabel, PROGRAM_NAME_WITH_VERSION, NULL);
  }


  XtRealizeWidget(toplevel);
  SetWindowManagerHint(TRUE);

  if (!remake && strlen(appres.geometry) == 0) {
    Window root;
    int x1, y1;
    unsigned int wd, ht, bd, dp;
    int max_wd, max_ht;

    XGetGeometry(dpy, XtWindow(toplevel), &root, &x1, &y1, &wd, &ht, &bd, &dp);
    max_wd = XtScreen(toplevel)->width * appres.max_width_ratio;
    max_ht = XtScreen(toplevel)->height * appres.max_height_ratio;
    if (appres.debug)
      fprintf(stderr, "window size: %dx%d, max size: %dx%d\n", wd, ht, max_wd, max_ht);
    if (max_wd < wd || max_ht < ht) {
      if (max_wd < wd) wd = max_wd;
      if (max_ht < ht) ht = max_ht;
      if (appres.debug)
	fprintf(stderr, "setting window size: %dx%d\n", wd, ht);
      XResizeWindow(dpy, XtWindow(toplevel), wd, ht);
    }
  }

  if (!appres.debug && key_box[first_row] != None) {
    if (appres.keypad) {
      XtVaGetValues(key_box[1], XtNwidth, &max_wd, NULL);
    } else {
      max_wd = 0;
      for (row = first_row; row < NUM_KEY_ROWS && key_box[row] != None; row++) {
        XtVaGetValues(key_box[row], XtNwidth, &wd, NULL);
        if (max_wd < wd) max_wd = wd;
      }
    }
    for (row = first_row; row < NUM_KEY_ROWS && key_box[row] != None; row++) {
      XtVaSetValues(key_box[row], XtNwidth, max_wd, NULL);
    }
  }
  if (0 < strlen(appres.geometry)) {
    if (appres.wm_toolbar) {
      if (appres.debug)
	fprintf(stderr, "window fgeometry ignored; _NET_WM_WINDOW_TYPE_TOOLBAR set on\n");
    } else {
      if (appres.debug)
	fprintf(stderr, "setting window geometry: %s\n", appres.geometry);
      XtVaSetValues(toplevel, XtNgeometry, appres.geometry, NULL);
      XtUnrealizeWidget(toplevel);
      XtRealizeWidget(toplevel);
    }
  }

  ReadKeymap();
  if (main_menu != None) RefreshMainMenu();
  RefreshShiftState(FALSE);

  XtMapWidget(toplevel);

  if (wm_delete_window == None)
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", FALSE);
  XSetWMProtocols(dpy, XtWindow(toplevel), &wm_delete_window, 1);

  XtVaGetValues(toplevel, XtNheight, &toplevel_height, NULL);
}

/*
 * WM_DELETE_WINDOW has been sent - terminate the program.
 */
static void DeleteWindowProc(Widget w, XEvent *event,
                             String *pars, Cardinal *n_pars)
{
  if (appres.nonexitable) {
    XBell(dpy, 0);
  } else {
    shift_state = 0;
    RefreshShiftState(TRUE);
    XtDestroyApplicationContext(XtWidgetToApplicationContext(toplevel));
    exit(0);
  }
}

/*
 * Callback for ConfigureNotify event, which will be invoked when
 * the toplevel window is resized.
 * We may need to switch the keytop labels when window becomes
 * smaller than appres.modal_threshold, and vice versa.
 */
static void WindowResized(Widget w, XEvent *event,
			  String *pars, Cardinal *n_pars)
{
  Dimension ht;

  XtVaGetValues(toplevel, XtNheight, &ht, NULL);
  if (appres.modal_threshold <= ht) {
    if (toplevel_height < appres.modal_threshold) MakeKeyboard(TRUE);
  } else {
    toplevel_height = ht;
  }
  RefreshShiftState(TRUE);
}

/*
 * Load list of text to be assigned to function keys.
 * Each line contains name of the key (with optional modifier)
 * and the text to be assigned to the key, as:
 *
 *   F1 text for F1
 *   s:F2 text for Shift-F2
 */
#ifndef PATH_MAX
# define PATH_MAX 300
#endif

static char fkey_filename[PATH_MAX] = "";

static struct fkey_struct {
  struct fkey_struct *next;
  char *value;
} *fkey_list = NULL;

static void ReadFuncionKeys(void)
{
  FILE *fp;
  char str[200], key[200];
  struct fkey_struct *sp = NULL, *new_node;
  char len;
  int val;
  const char *home;

  /* If KeyFile is not started with "/", consider the filename is relative to $HOME */
  /* and put value of the $HOME environment variable before the KeyFile. */
  /* To avoid possible buffer overflow, $HOME will not be added when resulting filename */
  /* is too long. */
  home = getenv("HOME");
  if (appres.key_file[0] != '/' && home != NULL
      && strlen(home) + strlen(appres.key_file) + 1 < sizeof(fkey_filename))
    sprintf(fkey_filename, "%s/%s", home, appres.key_file);
  else
    strncpy(fkey_filename, appres.key_file, sizeof(fkey_filename));

  strncpy(dict_filename, appres.dict_file, sizeof(dict_filename));

  fp = fopen(fkey_filename, "r");
  if (fp == NULL) return;

  while (fgets(str, sizeof(str) - 1, fp)) {
    if (str[0] == '#') {
      sscanf(&str[1], "%s %d", key, &val);
      if (strcmp(key, "quick_modifiers") == 0)
	appres.quick_modifiers = val;
      else if (strcmp(key, "shift_lock") == 0)
	appres.shift_lock = val;
      else if (strcmp(key, "altgr_lock") == 0)
	appres.altgr_lock = val;
      else if (strcmp(key, "modifiers_lock") == 0)
	appres.modifiers_lock = val;
      else if (strcmp(key, "key_click") == 0)
	appres.key_click_duration = val;
      else if (strcmp(key, "autoclick") == 0)
	appres.autoclick_delay = val;
      else if (strcmp(key, "always_on_top") == 0)
	appres.always_on_top = val;
      else if (strcmp(key, "wm_toolbar") == 0)
	appres.wm_toolbar = val;
      else if (strcmp(key, "jump_pointer") == 0)
	appres.jump_pointer = val;
      else if (strcmp(key, "dict_file") == 0) {
	sscanf(&str[1], "%*s %s", &key);
	strncpy(dict_filename, key, sizeof(dict_filename));
      }
    } else if (isalpha(str[0])) {
      len = strlen(str);
      if (str[len - 1] == '\n') str[len - 1] = '\0';

      new_node = malloc(sizeof(struct fkey_struct));
      if (fkey_list == NULL) fkey_list = new_node;
      else sp->next = new_node;
      sp = new_node;

      sp->next = NULL;
      sp->value = XtNewString(str);
    }
  }
  fclose(fp);
}

/*
 * Edit string assigned for function keys.
 * Modifiers (Shift, Ctrl, etc.) can't be handled here.
 */
static Widget edit_fkey_panel = None;
static Widget fkey_menu_button = None;
static Widget fkey_value_menu_button = None;
static Widget fkey_value_entry = None;
static char fkey_value[100] = "";
static char cur_fkey[20] = "";
static char *cur_fkey_value_mode = "";

static void FKeyValueMenuSelected(Widget w, char *key)
{
  char *key1, *cp;

  if (key[0] == 'c') {
    cur_fkey_value_mode = "command";
    key1 = "*command";
  } else {
    cur_fkey_value_mode = "string";
    key1 = "*string";
  }
  XtVaGetValues(XtNameToWidget(fkey_value_menu_button, key1), XtNlabel, &cp, NULL);
  XtVaSetValues(fkey_value_menu_button, XtNlabel, cp, NULL);
}

static void FKeyMenuSelected(Widget w, char *key)
{
  struct fkey_struct *sp, *sp2;
  int len;
  const char *value, *prefix;
  char key2[20];

  if (key == NULL)
    strcpy(key2, "");
  else if (strncmp(key, "Shift-", strlen("Shift-")) == 0)
    sprintf(key2, "s:%s", &key[strlen("Shift-")]);
  else
    strcpy(key2, key);

  if (strcmp(cur_fkey, key2) != 0) {
    if (strlen(cur_fkey) != 0) {
      len = strlen(cur_fkey);
      sp2 = NULL;
      for (sp = fkey_list; sp != NULL; sp = sp->next) {
	if (strncmp(sp->value, cur_fkey, len) == 0 && isspace(sp->value[len]))
	  break;
	sp2 = sp;
      }
      if (strlen(fkey_value) != 0) {  /* assign new string for the function key */
	if (sp == NULL) {  /* it was not defined before now */
	  sp = malloc(sizeof(struct fkey_struct));
	  if (fkey_list == NULL) fkey_list = sp;
	  else sp2->next = sp;
	  sp->next = NULL;
	  sp->value = NULL;
	}
	sp->value = realloc(sp->value, len + strlen(fkey_value) + 5);
	prefix = "";
	if (cur_fkey_value_mode[0] == 'c') prefix = "!";
	else if (fkey_value[0] == '!') prefix = "\\";
	sprintf(sp->value, "%s %s%s", cur_fkey, prefix, fkey_value);
      } else {  /* empty string - remove the entry for the function key */
	if (sp != NULL) {
	  if (sp2 != NULL) sp2->next = sp->next;
	  else fkey_list = sp->next;
	  free(sp->value);
	  free(sp);
	}
      }
    }

    if (key != NULL) {
      XtVaSetValues(fkey_menu_button, XtNlabel, key, NULL);
  
      value = FindFunctionKeyValue(key2, FALSE);
      if (value == NULL) value = "";

      FKeyValueMenuSelected(None, (value[0] == '!') ? "command" : "string");

      if (value[0] == '!' || value[0] == '\\') value = value + 1;
      strncpy(fkey_value, value, sizeof(fkey_value) - 1);
      XtVaSetValues(fkey_value_entry, XtNstring, fkey_value, NULL);

      strcpy(cur_fkey, key2);
    }
  }
}

static void CloseFunctionKeyPanel(void)
{
  XtPopdown(edit_fkey_panel);
}

static void SaveFunctionKey(void)
{
  struct fkey_struct *sp;
  FILE *fp;

  if (appres.debug) fprintf(stderr, "SaveFunctionKey\n");

  if (edit_fkey_panel != None) FKeyMenuSelected(None, NULL);

  fp = fopen(fkey_filename, "w");
  if (fp == NULL) {
    fprintf(stderr, "%s: can't open \"%s\": %s\n",
	    PROGRAM_NAME, fkey_filename, strerror(errno));
    return;
  }
  fprintf(fp, "#quick_modifiers %d\n", appres.quick_modifiers);
  fprintf(fp, "#shift_lock %d\n", appres.shift_lock);
  fprintf(fp, "#altgr_lock %d\n", appres.altgr_lock);
  fprintf(fp, "#modifiers_lock %d\n", appres.modifiers_lock);
  fprintf(fp, "#key_click %d\n", appres.key_click_duration);
  fprintf(fp, "#autoclick %d\n", appres.autoclick_delay);
  fprintf(fp, "#always_on_top %d\n", appres.always_on_top);
  fprintf(fp, "#wm_toolbar %d\n", appres.wm_toolbar);
  fprintf(fp, "#jump_pointer %d\n", appres.jump_pointer);
  fprintf(fp, "#dict_file %s\n", dict_filename);
  for (sp = fkey_list; sp != NULL; sp = sp->next) {
    fprintf(fp, "%s\n", sp->value);
  }
  fclose(fp);

  if (edit_fkey_panel != None) CloseFunctionKeyPanel();
}

static void PopupFunctionKeyEditor(void)
{
  Widget form, form2, menu, menu_entry, button;
  char label[20];
  char *key;
  int i, j;

  if (edit_fkey_panel == None) {
    edit_fkey_panel = XtVaCreatePopupShell("edit_fkey_panel", transientShellWidgetClass,
					   toplevel, NULL);
    form = XtVaCreateManagedWidget("form", formWidgetClass, edit_fkey_panel, NULL);

    form2 = XtVaCreateManagedWidget("form2", formWidgetClass, form, NULL);
    XtVaCreateManagedWidget("fkey_label", labelWidgetClass, form2, NULL);
    fkey_menu_button = XtVaCreateManagedWidget("fkey_menu", menuButtonWidgetClass,
					       form2, NULL);
    menu = XtVaCreatePopupShell("menu", simpleMenuWidgetClass, fkey_menu_button, NULL);
    for (j = 0; j <= 1; j++) {
      for (i = 1; i <= appres.editable_function_keys; i++) {
	if (j == 0)
	  sprintf(label, "F%d", i);
	else 
	  sprintf(label, "Shift-F%d", i);
	key = XtNewString(label);
	menu_entry = XtVaCreateManagedWidget(key, smeBSBObjectClass, menu, NULL);
	XtAddCallback(menu_entry, XtNcallback, (XtCallbackProc)FKeyMenuSelected,
		      (XtPointer)key);
      }
    }

    fkey_value_menu_button = XtVaCreateManagedWidget("fkey_value_menu", menuButtonWidgetClass,
						     form2, NULL);
    menu = XtVaCreatePopupShell("menu", simpleMenuWidgetClass, fkey_value_menu_button, NULL);
    menu_entry = XtVaCreateManagedWidget("string", smeBSBObjectClass, menu, NULL);
    XtAddCallback(menu_entry, XtNcallback, (XtCallbackProc)FKeyValueMenuSelected,
		  (XtPointer)"string");
    menu_entry = XtVaCreateManagedWidget("command", smeBSBObjectClass, menu, NULL);
    XtAddCallback(menu_entry, XtNcallback, (XtCallbackProc)FKeyValueMenuSelected,
		  (XtPointer)"command");

    XtVaCreateManagedWidget("fkey_value_sep", labelWidgetClass, form2, NULL);

    fkey_value_entry = XtVaCreateManagedWidget("fkey_value", asciiTextWidgetClass, form2,
					       XtNuseStringInPlace, True,
					       XtNeditType, XawtextEdit,
					       XtNstring, fkey_value,
					       XtNlength, sizeof(fkey_value) - 1,
					       NULL);

    button = XtVaCreateManagedWidget("save_button", commandWidgetClass, form, NULL);
    XtAddCallback(button, XtNcallback, (XtCallbackProc)SaveFunctionKey, NULL);

    button = XtVaCreateManagedWidget("close_button", commandWidgetClass, form, NULL);
    XtAddCallback(button, XtNcallback, (XtCallbackProc)CloseFunctionKeyPanel, NULL);

    XtRealizeWidget(edit_fkey_panel);
    XSetWMProtocols(dpy, XtWindow(edit_fkey_panel), &wm_delete_window, 1);

    XtSetKeyboardFocus(edit_fkey_panel, fkey_value_entry);

    FKeyMenuSelected(None, "F1");
  }

  XtPopup(edit_fkey_panel, XtGrabNone);
}

/*
 * If text is assigned to the specified function key,
 * return the text.  Otherwise, return NULL.
 */
static const char *FindFunctionKeyValue(const char *key, Boolean shiftable)
{
  char label[50];
  char prefix;
  struct fkey_struct *sp;
  int len;

  prefix = '\0';
  if (shiftable) {
    if (shift_state & meta_mask) prefix = 'm';
    else if (shift_state & alt_mask) prefix = 'a';
    else if (shift_state & ControlMask) prefix = 'c';
    else if (shift_state & ShiftMask) prefix = 's';
  }
  if (prefix == '\0') sprintf(label, "%s", key);
  else sprintf(label, "%c:%s", prefix, key);
  len = strlen(label);
  
  for (sp = fkey_list; sp != NULL; sp = sp->next) {
    if (strncmp(sp->value, label, len) == 0 && isspace(sp->value[len]))
      return &(sp->value[len + 1]);
  }
  return NULL;
}

/*
 * Key click
 */
void KeyClick(void)
{
  XKeyboardState ks;
  XKeyboardControl kc;

  if (0 < appres.key_click_duration) {
    XGetKeyboardControl(dpy, &ks);

    kc.bell_duration = ks.bell_duration;
    kc.bell_pitch = appres.key_click_pitch;
    kc.bell_duration = appres.key_click_duration;
    XChangeKeyboardControl(dpy, KBBellPitch | KBBellDuration, &kc);
    XBell(dpy, 0);
    XSync(dpy, FALSE);

    kc.bell_pitch = ks.bell_pitch;
    kc.bell_duration = ks.bell_duration;
    XChangeKeyboardControl(dpy, KBBellPitch | KBBellDuration, &kc);
    XSync(dpy, FALSE);
  }
}

/*
 * Display balloon message for the function keys,
 * if text is assigned to the key.
 */
static Boolean balloon_panel_open = FALSE;
static Widget balloon_panel = None;

static	XtIntervalId autoclick_id = (XtIntervalId)0;

static void StopAutoclick(void)
{
  if (autoclick_id != (XtIntervalId)0) {
    if (appres.debug) fprintf(stderr, "StopAutoclick: %lx\n", (long)autoclick_id);

    XtRemoveTimeOut(autoclick_id);
    autoclick_id = (XtIntervalId)0;
  }
}

static void Autoclick(void)
{
  StopAutoclick();

  XTestFakeButtonEvent(target_dpy, 1, True, CurrentTime);
  XTestFakeButtonEvent(target_dpy, 1, False, CurrentTime);
}

static void ShowBalloon(Widget w, XEvent *event, String *pars, Cardinal *n_pars)
{
  static Widget message;
  Position x, y;
  Dimension ht;
  const char *value;

  if (strcmp(XtName(w), "MainMenu") == 0) {
    value = "Main menu";
  } else {
    if (0 < appres.autoclick_delay) {
      autoclick_id = XtAppAddTimeOut(app_con, (long)appres.autoclick_delay,
			   (XtTimerCallbackProc)Autoclick, (XtPointer)w);

      if (appres.debug) fprintf(stderr, "ShowBalloon: auto click triggerd: %lx, %d\n",
				(long)autoclick_id, appres.autoclick_delay);
    }
    value = FindFunctionKeyValue(XtName(w), TRUE);
    if (value == NULL) return;
  }

  if (balloon_panel == None) {
    balloon_panel = XtVaCreatePopupShell("balloon_panel", transientShellWidgetClass, toplevel,
					 XtNoverrideRedirect, TRUE, NULL);
    message = XtVaCreateManagedWidget("message", labelWidgetClass, balloon_panel, NULL);
  }
  XtVaGetValues(w, XtNheight, &ht, NULL);
  XtTranslateCoords(w, 6, ht + 2, &x, &y);
  XtVaSetValues(balloon_panel, XtNx, x, XtNy, y, NULL);
  if (value[0] == '!') {
    if (appres.secure) return;
    XtVaSetValues(message, XtNlabel, value + 1,
		  XtNbackground, appres.launch_balloon_background, NULL);
  } else {
    if (value[0] == '\\') value = value + 1;
    XtVaSetValues(message, XtNlabel, value,
		  XtNbackground, appres.balloon_background, NULL);
  }
  XtPopup(balloon_panel, XtGrabNone);

  balloon_panel_open = TRUE;
}

static void CloseBalloon(Widget w, XEvent *event, String *pars, Cardinal *n_pars)
{
  StopAutoclick();
  if (balloon_panel_open) {
    XtPopdown(balloon_panel);
    balloon_panel_open = FALSE;
  }
}

/*
 * Set icon image.
 */
static void SetIconBitmap(Widget w)
{
#include "xvkbd_icon.xbm"
#include "xvkbd_iconmask.xbm"

  Pixmap icon_pixmap, icon_mask;

  icon_pixmap = XCreateBitmapFromData(XtDisplay(w), XtWindow(w),
				      (char *)xvkbd_icon_bits,
				      xvkbd_icon_width, xvkbd_icon_height);;
  icon_mask = XCreateBitmapFromData(XtDisplay(w), XtWindow(w),
				    (char *)xvkbd_iconmask_bits,
				    xvkbd_iconmask_width, xvkbd_iconmask_height);
  XtVaSetValues(w, XtNiconPixmap, icon_pixmap, XtNiconMask, icon_mask, NULL);
}

/*
 * Callback for VisibilityChanged event, which will be invoked
 * when xvkbd window is hidden by other window.  ** EXPERIMENTAL **
 */
static void VisibilityChanged(Widget w, XEvent *event,
			      String *pars, Cardinal *n_pars)
{
  static cnt = 0;
  static time_t t1 = 0;
  time_t t2;

  if (!appres.always_on_top) return;

  if (balloon_panel_open) return;

  if (main_menu != None && XtWindow(main_menu) != None) {
    XWindowAttributes attr;
    XGetWindowAttributes(dpy, XtWindow(main_menu), &attr);
    if (attr.map_state != IsUnmapped) return;
  }

  t2 = time(NULL);
  if (t1 != t2) cnt = 0;
  t1 = t2;
  cnt = cnt + 1;
  if (appres.debug)
    fprintf(stderr, "%s: visibility of the window changed (cnt = %d)\n", PROGRAM_NAME, cnt);
  if (cnt < 5)
    XRaiseWindow(XtDisplay(toplevel), XtWindow(toplevel));
}

/*
 * The main program.
 */
int main(int argc, char *argv[])
{
  static XtActionsRec actions[] = {
    { "DeleteWindowProc", DeleteWindowProc },
    { "WindowResized", WindowResized },
    { "VisibilityChanged", VisibilityChanged },
    { "ReadKeymap", (XtActionProc)ReadKeymap },
    { "ButtonDownAction", ButtonDownAction },
    { "ButtonUpAction", ButtonUpAction },
    { "ShowBalloon", ShowBalloon },
    { "CloseBalloon", CloseBalloon },
    { "ClosePopupPanel", (XtActionProc)ClosePopupPanel },
  };
  static String fallback_resources[] = {
#include "XVkbd-common.h"
    NULL,
  };

  Boolean open_keypad_panel = FALSE;
  char ch;
  Window child;
  int op, ev, err;

  argc1 = argc;
  argv1 = malloc(sizeof(char *) * (argc1 + 5));
  memcpy(argv1, argv, sizeof(char *) * argc1);
  argv1[argc1] = NULL;

#ifdef USE_I18N
  XtSetLanguageProc(NULL, NULL, NULL);
#endif

  toplevel = XtVaAppInitialize(NULL, "XVkbd",
                               options, XtNumber(options),
                               &argc, argv, fallback_resources, NULL);
  dpy = XtDisplay(toplevel);
  app_con = XtWidgetToApplicationContext(toplevel);
  XtAppAddActions(app_con, actions, XtNumber(actions));

  target_dpy = dpy;

  if (1 < argc) {
    fprintf(stderr, "%s: illegal option: %s\n\n", PROGRAM_NAME, argv[1]);
  }

  XtGetApplicationResources(toplevel, &appres,
            application_resources, XtNumber(application_resources),
            NULL, 0);
  if (appres.version) {
    fprintf(stdout, "%s\n", appres.description);
    exit(1);
  }

  if (appres.compact) {
    appres.keypad = FALSE;
    appres.function_key = FALSE;
  }
  if (appres.keypad_only && !appres.keypad) {
    appres.keypad_only = FALSE;
    open_keypad_panel = TRUE;
  }

  if (appres.no_sync) {
    XSync(dpy, FALSE);
    XSetErrorHandler(MyErrorHandler);
  }
  
  if (0 < strlen(appres.window) || 0 < strlen(appres.instance)) {
    if (strcmp(appres.window, "root") == 0) {
      focused_window = RootWindow(dpy, DefaultScreen(dpy));
    } else if (sscanf(appres.window, "0x%lX%c", &focused_window, &ch) != 1) {
      if (sscanf(appres.window, "%ld%c", &focused_window, &ch) != 1) {
        focused_window = FindWindow(RootWindow(dpy, DefaultScreen(dpy)),
                                    appres.window);
        if (focused_window == None) {
          fprintf(stderr, "%s: no such window: window=%s and class=%s\n", PROGRAM_NAME, appres.window, appres.instance);
	  if (appres.no_root)
	    exit(-1);
        }
      }
    }
  }
  focused_subwindow = focused_window;

  ReadKeymap();
  if (!altgr_mask && appres.auto_add_keysym) AddModifier(XK_Mode_switch);

  if (strlen(appres.text) != 0 || strlen(appres.file) != 0) {
    appres.keypad_keysym = TRUE;
    if (focused_window != None &&
        (appres.list_widgets || strlen(appres.widget) != 0)) {
      XtVaSetValues(toplevel, XtNwidth, 1, XtNheight, 1, NULL);
      XtRealizeWidget(toplevel);
      child = FindWidget(toplevel, focused_window, appres.widget);
      if (child != None) focused_subwindow = child;
    }
    if (strlen(appres.text) != 0)
      SendString(appres.text);
    else
      SendFileContent(appres.file);
    exit(0);
 } else {
    ReadFuncionKeys();

    if (0 < strlen(appres.keys_normal))
      RedefineKeys(keys_normal, appres.keys_normal);
    if (0 < strlen(appres.keys_shift))
      RedefineKeys(keys_shift, appres.keys_shift);
    if (0 < strlen(appres.keys_altgr))
      RedefineKeys(keys_altgr, appres.keys_altgr);
    if (0 < strlen(appres.keys_shift_altgr))
      RedefineKeys(keys_shift_altgr, appres.keys_shift_altgr);

    if (0 < strlen(appres.key_labels))
      RedefineKeys(key_labels, appres.key_labels);
    if (0 < strlen(appres.normal_key_labels))
      RedefineKeys(normal_key_labels, appres.normal_key_labels);
    if (0 < strlen(appres.shift_key_labels))
      RedefineKeys(shift_key_labels, appres.shift_key_labels);
    if (0 < strlen(appres.altgr_key_labels))
      RedefineKeys(altgr_key_labels, appres.altgr_key_labels);
    if (0 < strlen(appres.shift_altgr_key_labels))
      RedefineKeys(shift_altgr_key_labels, appres.shift_altgr_key_labels);

    if (0 < strlen(appres.keypad_normal)) {
      RedefineKeys(keypad, appres.keypad_normal);
      RedefineKeys(keypad_shift, appres.keypad_normal);
      RedefineKeys(keypad_label, appres.keypad_normal);
    }
    if (0 < strlen(appres.keypad_shift))
      RedefineKeys(keypad_shift, appres.keypad_shift);
    if (0 < strlen(appres.keypad_labels))
      RedefineKeys(keypad_label, appres.keypad_labels);

    MakeKeyboard(FALSE);

    if (focused_window != None &&
        (appres.list_widgets || strlen(appres.widget) != 0)) {
      child = FindWidget(toplevel, focused_window, appres.widget);
      if (child != None) focused_subwindow = child;
    }

    if (main_menu != None) {
      if (strlen(dict_filename) == 0)
	XtSetSensitive(XtNameToWidget(main_menu, "*completion"), FALSE);
      if (strlen(appres.customizations) == 0)
	XtSetSensitive(XtNameToWidget(main_menu, "*select_layout"), FALSE);
      if (appres.nonexitable)
	XtSetSensitive(XtNameToWidget(main_menu, "*quit"), FALSE);
      if (appres.secure) {
	XtSetSensitive(XtNameToWidget(main_menu, "*man"), FALSE);
	XtSetSensitive(XtNameToWidget(main_menu, "*open_display"), FALSE);
      }
    }

#ifdef USE_XTEST
    if (!XQueryExtension(dpy, "XTEST", &op, &ev, &err)) {
      if (appres.xtest) {
	fprintf(stderr, "%s: XTEST extension is not supported by the X server\n",
		PROGRAM_NAME);
	fprintf(stderr, "%s: XSendEvent will be used instead\n",
		PROGRAM_NAME);
	appres.xtest = FALSE;
      }
      if (main_menu != None) {
	XtSetSensitive(XtNameToWidget(main_menu, "*use_xtest"), FALSE);
	RefreshMainMenu();
      }
    }
#endif

    if (!appres.debug) {
#ifdef SYSV
      signal(SIGINT, SIG_IGN);
      signal(SIGQUIT, SIG_IGN);
#else
      struct sigaction sigact;
      sigact.sa_handler = SIG_IGN;
      sigemptyset(&sigact.sa_mask);
      sigact.sa_flags = 0;
      sigaction(SIGINT, &sigact, NULL);
      sigaction(SIGQUIT, &sigact, NULL);
#endif
    }

    {
#ifdef SYSV
      signal(SIGUSR1, SignalUser1);
#else
      struct sigaction sigact;
      sigact.sa_handler = SignalUser1;
      sigemptyset(&sigact.sa_mask);
      sigact.sa_flags = 0;
      sigaction(SIGUSR1, &sigact, NULL);
#endif
    }

    SetIconBitmap(toplevel);

    if (open_keypad_panel) MenuSelected(None, "keypad");


    XtAppMainLoop(app_con);
  }
  exit(0);
}

/*
 * Replace setlocale() in the standard library here, because
 * it may not support some locales used for localized keyboards.
 */
#if defined(USE_I18N) && !defined(HAVE_SETLOCALE)

char *setlocale(int category, const char *locale)
{
  static char old_locale[100] = "C";
  static char cur_locale[100] = "C";
  const char *s;
  if (locale == NULL) {
    return cur_locale;
  } else if (category == LC_ALL) {
    strcpy(old_locale, cur_locale);
    if (locale[0] == '\0') {
      s = getenv("LC_ALL");
      if (s == NULL) s = "C";  /* LC_ALL not defined */
    } else {
      s = locale;
    }
    strncpy(cur_locale, s, sizeof(cur_locale) - 1);
    return old_locale;
  } else {
    return cur_locale;
  }
}
#endif  /* HAVE_SETLOCALE */
