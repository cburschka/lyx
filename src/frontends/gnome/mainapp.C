// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *       
 *          Copyright 2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>
#include <gnome--/main.h>
#include <gtk--/accelgroup.h>
#include <vector>
#include <algorithm>

#include "mainapp.h"

using SigC::bind;
using SigC::slot;

GLyxAppWin::GLyxAppWin() :
  Gnome::App("GnomeLyX","LyX Gnomified"),
	 status_(false, true, GNOME_PREFERENCES_NEVER)
{
  init();
  show_all();
}

GLyxAppWin::~GLyxAppWin()
{
}

void GLyxAppWin::init()
{
  // set defaults
  set_policy(false, true, false);
  set_default_size(250, 350);
  set_wmclass("lyx", "GnomeLyX");

  frame_.set_shadow_type(GTK_SHADOW_IN);

  set_contents(frame_);

  set_statusbar(status_);

  // initial (dummy) menu
  vector<Gnome::UI::Info> menus, fm;
  fm.push_back(Gnome::MenuItems::Open());
  menus.push_back(Gnome::Menus::File(fm));

  Gnome::UI::Array menu = menus;
  gnome_app_create_menus(this->gtkobj(),
			 menu.gtkobj());

  menusize_ = menu.size();
}


void GLyxAppWin::set_menu(Gnome::UI::Array &menu)
{
  // clean up and install new menus
  gnome_app_remove_menus(this->gtkobj(),"/",menusize_);
  gnome_app_insert_menus(this->gtkobj(), "", menu.gtkobj());
  gnome_app_install_menu_hints(this->gtkobj(), menu.gtkobj());
  menusize_ = menu.size();
}


