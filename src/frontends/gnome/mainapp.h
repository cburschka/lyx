// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *       
 *          Copyright 2000 The LyX Team.
 *
 * ====================================================== */

#ifndef GNOMELYX_MAINWIN
#define GNOMELYX_MAINWIN

#include <gnome--/app.h>
#include <gnome--/appbar.h>
#include <gtk--/frame.h>

#include "MenuBackend.h"

/*
 This is (most probably) temporary class and it will be either merged with
 LyXView or replaced by new implementation of GUI-dependend MainWindow class
*/

class GLyxAppWin: public Gnome::App
{
 public:
  GLyxAppWin();
  ~GLyxAppWin();

  /// set menu of the window
  void set_menu(Gnome::UI::Array &);
  /// update menu
  void update_menu(string path,
		   int noelms,
		   Gnome::UI::Array &);
  /// add action area
  void add_action(Gtk::Container &, string title, bool expand=false);
  /// remove action area
  void remove_action();
  /// clears action area if Escape is pressed
  gint key_pressed(GdkEventKey * e);
  
 protected:
  /// init window widgets
  void init();

 protected:
  // widgets
  Gnome::AppBar status_;
  Gtk::VBox box_;
  Gtk::Widget *view_;

  bool action_mode;
  
  // menu size
  int menusize_;
};

#endif
