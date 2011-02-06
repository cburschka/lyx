/*
 * xvkbd - Virtual Keyboard for X Window System
 *
 * Copyright (C) 2000 by Tom Sato <VEF00200@nifty.ne.jp>
 * http://homepage3.nifty.com/tsato/
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

#ifndef PROGRAM_NAME
# define PROGRAM_NAME "xvkbd"
#endif

struct appres_struct {
  String description;
  String show_manual_command;

  String geometry;
  Boolean inherit_geometry;
  Boolean version;
  Boolean debug;
  Boolean xtest;
  Boolean no_sync;
  Boolean always_on_top;
  Boolean wm_toolbar;
  Boolean jump_pointer;
  Boolean jump_pointer_always;
  Boolean jump_pointer_back;
  Boolean quick_modifiers;
  Boolean altgr_lock;
  Boolean shift_lock;
  Boolean modifiers_lock;
  Boolean num_lock_state;
  Boolean auto_repeat;
  Boolean modal_keytop;
  Boolean minimizable;
  Boolean secure;
  Boolean no_root;
  Boolean nonexitable;
  int modal_threshold;
  Boolean keypad;
  Boolean function_key;
  Boolean compact;
  Boolean keypad_only;
  Boolean keypad_keysym;
  Boolean auto_add_keysym;
  Boolean list_widgets;
  String positive_modifiers;
  String text;
  String file;
  String window;
  String instance;
  String widget;
  String wait_idle;
  XFontStruct *general_font;
  XFontStruct *letter_font;
  XFontStruct *special_font;
  XFontStruct *keypad_font;
#ifdef USE_I18N
  XFontSet special_fontset;
#endif
  Pixel general_background;
  Pixel special_background;
  Pixel special_foreground;
  Pixel highlight_background;
  Pixel highlight_foreground;
  Pixel focus_background;
  Pixel remote_focus_background;
  Pixel balloon_background;
  Pixel launch_balloon_background;

  String keys_normal;
  String keys_shift;
  String keys_altgr;
  String keys_shift_altgr;
  String key_labels;
  String normal_key_labels;
  String shift_key_labels;
  String altgr_key_labels;
  String shift_altgr_key_labels;

  String keypad_normal;
  String keypad_shift;
  String keypad_labels;

  String deadkeys;
  int altgr_keycode;

  String key_file;
  String dict_file;
  String customizations;
  int editable_function_keys;

  float max_width_ratio;
  float max_height_ratio;
  int text_delay;
  int key_click_pitch;
  int key_click_duration;
  int autoclick_delay;
} appres;


/* FindWidget() is defined in findwidget.c */
extern Window FindWidget(Widget w, Window client, const char *name);
