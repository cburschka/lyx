// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "gettext.h"
#include "Dialogs.h"
#include "FormError.h"
#include "LyXView.h"
#include "buffer.h"
#include "lyxfunc.h"

#include <gtk--/label.h>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/buttonbox.h>
#include <gnome--/stock.h>
#include <gtk--/separator.h>
#include <gtk--/alignment.h>

// temporary solution for LyXView
#include "mainapp.h"
extern GLyxAppWin * mainAppWin;


FormError::FormError(LyXView * lv, Dialogs * d)
	: lv_(lv), d_(d), inset_(0), u_(0), h_(0), ih_(0), dialog_(NULL)
{
  // let the dialog be shown
  // These are permanent connections so we won't bother
  // storing a copy because we won't be disconnecting.
  d->showError.connect(slot(this, &FormError::showInset));
}


FormError::~FormError()
{
  hide();
}

void FormError::showInset( InsetError * const inset )
{
  if( dialog_!=NULL || inset == 0 ) return;
  
  inset_ = inset;
  ih_ = inset_->hide.connect(slot(this, &FormError::hide));

  show();
}

void FormError::show()
{
  if (!dialog_)
    {
      using namespace Gtk::Box_Helpers;
      
      Gtk::Label * label = manage( new Gtk::Label(inset_->getContents()) );
      Gtk::Box * hbox = manage( new Gtk::HBox() );
      Gtk::Button * b_close = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_CLOSE) ) );
      Gtk::Alignment * alg1 = manage( new Gtk::Alignment(0.5, 0.5, 0, 0) );
      Gtk::Alignment * mbox = manage( new Gtk::Alignment(0.5, 0.5, 0, 0) );
      
      // set up spacing
      hbox->set_spacing(4);

      // packing
      alg1->add(*b_close);
      
      hbox->children().push_back(Element(*label, false, false));
      hbox->children().push_back(Element(*manage(new Gtk::VSeparator()), false, false));
      hbox->children().push_back(Element(*alg1, false, false));

      mbox->add(*hbox);
      
      // packing dialog to main window
      dialog_ = mbox;
      mainAppWin->add_action(*dialog_, N_(" Error "));

      // setting focus
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(b_close->gtkobj()), GTK_CAN_DEFAULT);
      gtk_widget_grab_focus (GTK_WIDGET(b_close->gtkobj()));
      gtk_widget_grab_default (GTK_WIDGET(b_close->gtkobj()));

      // connecting signals
      b_close->clicked.connect(slot(mainAppWin, &GLyxAppWin::remove_action));
      dialog_->destroy.connect(slot(this, &FormError::free));

      u_ = d_->updateBufferDependent.connect(slot(this, &FormError::update));
      h_ = d_->hideBufferDependent.connect(slot(this, &FormError::hide));
    }
}
      
void FormError::update()
{
}

void FormError::hide()
{
  if (dialog_!=NULL) mainAppWin->remove_action();
}

void FormError::free()
{
  if (dialog_!=NULL)
    {
      dialog_ = NULL;
      u_.disconnect();
      h_.disconnect();
      inset_ = 0;
      ih_.disconnect();
    }
}

void FormError::apply()
{
}

