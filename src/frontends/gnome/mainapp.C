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
#include <gnome--/pixmap.h>
#include <gtk--/separator.h>
#include <gtk--/frame.h>
#include <gtk--/label.h>

#include <vector>
#include <algorithm>

#include "mainapp.h"

using SigC::bind;
using SigC::slot;

GLyxAppWin::GLyxAppWin() :
  Gnome::App(PACKAGE,"LyX Gnomified"),
	 status_(false, true, GNOME_PREFERENCES_NEVER),
	 action_mode(false)
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
  set_wmclass(PACKAGE, "GnomeLyX");

  set_statusbar(status_);

  accel_ = 0;

  // initial (dummy) menu
  vector<Gnome::UI::Info> menus, fm;
  fm.push_back(Gnome::MenuItems::Open());
  menus.push_back(Gnome::Menus::File(fm));

  GLyxAppWin::Array menu = menus;
  gnome_app_create_menus(this->gtkobj(),
			 menu.gtkobj());

  menusize_ = menu.size();

  // packing widgets

  // temporary main widget
  Gtk::HBox * h = manage( new Gtk::HBox() );
  Gnome::Pixmap * p;
  p = Gtk::wrap( GNOME_PIXMAP( gnome_stock_pixmap_widget(0, GNOME_STOCK_PIXMAP_ABOUT) ) );

  h->children().push_back( Gtk::Box_Helpers::Element( *p ) );
  h->children().push_back( *(manage(new Gtk::Label("Waiting for LyXView port"))) );

  view_ = h;
  // temporary main widget: done

  // packing main widget and separator
  Gtk::Separator * sep = manage( new Gtk::HSeparator() );

  box_.children().push_back( Gtk::Box_Helpers::Element(*view_) );
  box_.children().push_back( Gtk::Box_Helpers::Element(*sep, false) );
  
  box_.show_all();
  
  set_contents(box_);

  key_press_event.connect(slot(this, &GLyxAppWin::key_pressed));
}


void GLyxAppWin::set_menu(Array &menu)
{
  // clean up and install new menus
  gnome_app_remove_menus(this->gtkobj(),"/",menusize_);
  gnome_app_insert_menus(this->gtkobj(), "", menu.gtkobj());
  gnome_app_install_menu_hints(this->gtkobj(), menu.gtkobj());
  menusize_ = menu.size();
}

void GLyxAppWin::update_menu(string path, int noelms, Array &menu)
{
  // remove "noelms" items and install new items from "menu"
  gnome_app_remove_menus(this->gtkobj(),path.c_str(),noelms);
  gnome_app_insert_menus(this->gtkobj(),path.c_str(),menu.gtkobj());
  gnome_app_install_menu_hints(this->gtkobj(),menu.gtkobj());
}
  
// clean up first, then add new action widget and finally, disable main view
void GLyxAppWin::add_action(Gtk::Container &action, string title, bool expand, Gtk::AccelGroup * acgr)
{
  remove_action();

  Gtk::Frame * frame = manage( new Gtk::Frame(title) );
  frame->set_border_width(2);
  action.set_border_width(2);
  frame->add(action);
  
  box_.children().push_back( Gtk::Box_Helpers::Element( *frame, expand ) );
  box_.show_all();

  accel_ = acgr;
  if (accel_ != 0) add_accel_group(*accel_);
  
  view_->set_sensitive(false);
  action_mode = true;
}

void GLyxAppWin::remove_action()
{
  if (accel_ != 0)
    {
      remove_accel_group(*accel_);
      accel_ = 0;
    }
  
  while ( box_.children().size() > 2 )
    {
      box_.children().pop_back();
    }

  view_->set_sensitive(true);  
  action_mode = false;
}

gint GLyxAppWin::key_pressed(GdkEventKey * e)
{
  if (action_mode &&
      e->keyval == GDK_Escape)
    {
      remove_action();
      return TRUE;
    }
  return FALSE;
}

