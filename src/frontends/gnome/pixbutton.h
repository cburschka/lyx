// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *       
 *          Copyright 2000 The LyX Team.
 *
 * ====================================================== */

#ifndef GNOME_PIXBUTTON
#define GNOME_PIXBUTTON

#include <gtk--/button.h>
#include <gtk--/box.h>
#include <gtk--/label.h>
#include <gnome--/pixmap.h>
#include <gnome--/stock.h>

/* Button class used in LyX Gnome frontend for buttons with pixmaps and
   accelerators */

namespace Gnome
{
  class PixButton: public Gtk::Button
  {
  public:
    PixButton(string label, string pixname): Button()
    {
      Gtk::Box * b = manage( new Gtk::HBox() );
      Gtk::Label * l = manage( new Gtk::Label(label) );
      Gnome::Pixmap * p = Gtk::wrap( GNOME_PIXMAP( gnome_stock_pixmap_widget(NULL, pixname.c_str()) ) );

      b->set_spacing(3);
      b->children().push_back(Gtk::Box_Helpers::Element(*p, false, false));
      b->children().push_back(Gtk::Box_Helpers::Element(*l, false, false));

      add(*b);

      accelkey_ = l->parse_uline(label);
      
      l->show();
      p->show();
      b->show();
    }

    guint get_accelkey() { return accelkey_; }

  protected:
    guint accelkey_;
  };
}

#endif

