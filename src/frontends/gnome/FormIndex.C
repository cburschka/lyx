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
#include "FormIndex.h"
#include "LyXView.h"
#include "buffer.h"
#include "lyxfunc.h"

#include "gettext.h"
#include "Dialogs.h"
#include "FormIndex.h"
#include "LyXView.h"
#include "buffer.h"
#include "form_index.h"
#include "lyxfunc.h"

#include <gtk--/label.h>
#include <gtk--/box.h>
#include <gtk--/buttonbox.h>
#include <gnome--/entry.h>
#include <gnome--/stock.h>
#include <gtk--/separator.h>

// temporary solution for LyXView
#include "mainapp.h"
extern GLyxAppWin * mainAppWin;

// configuration keys
static string const CONF_ENTRY("FormIndex_entry");

FormIndex::FormIndex(LyXView * lv, Dialogs * d)
	: lv_(lv), d_(d), inset_(0), u_(0), h_(0), ih_(0), dialog_(0)
{
  // let the dialog be shown
  // These are permanent connections so we won't bother
  // storing a copy because we won't be disconnecting.
  d->showIndex.connect(slot(this, &FormIndex::showInset));
  d->createIndex.connect(slot(this, &FormIndex::createInset));
}


FormIndex::~FormIndex()
{
  hide();
}

void FormIndex::showInset( InsetCommand * const inset )
{
  if( dialog_!=0 || inset == 0 ) return;
  
  inset_ = inset;
  ih_ = inset_->hide.connect(slot(this, &FormIndex::hide));
  
  params = inset->params();
  show();
}

void FormIndex::createInset( string const & arg )
{
  if( dialog_!=0 ) return;
  
  params.setFromString( arg );
  show();
}

void FormIndex::show()
{
  if (!dialog_)
    {
      using namespace Gtk::Box_Helpers;
      
      Gtk::Label * label = manage( new Gtk::Label(_("Keyword")) );
      Gtk::Box * mbox = manage( new Gtk::HBox() );
      Gtk::ButtonBox * bbox = manage( new Gtk::HButtonBox() );
      Gtk::Separator * sep = manage( new Gtk::VSeparator() );

      keyword_ = manage( new Gnome::Entry() );
      
      b_ok = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_OK) ) );
      b_cancel = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL) ) );
      
      // set up spacing
      mbox->set_spacing(2);
      bbox->set_spacing(4);

      keyword_->set_history_id(CONF_ENTRY);
      keyword_->set_max_saved(10);
      keyword_->load_history();
      keyword_->set_use_arrows_always(true);
      
      // packing
      bbox->children().push_back(Element(*b_ok, false, false));
      bbox->children().push_back(Element(*b_cancel, false, false));

      mbox->children().push_back(Element(*label, false, false));
      mbox->children().push_back(Element(*keyword_, true, true));
      mbox->children().push_back(Element(*sep, false, false));
      mbox->children().push_back(Element(*bbox, false, false));

      // packing dialog to main window
      dialog_ = mbox;
      mainAppWin->add_action(*dialog_, _(" Index "));

      // setting focus
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(keyword_->get_entry()->gtkobj()), GTK_CAN_DEFAULT);
      gtk_widget_grab_focus (GTK_WIDGET(keyword_->get_entry()->gtkobj()));
      gtk_widget_grab_default (GTK_WIDGET(keyword_->get_entry()->gtkobj()));

      // connecting signals
      b_ok->clicked.connect(slot(this, &FormIndex::apply));
      keyword_->get_entry()->activate.connect(slot(this, &FormIndex::apply));

      b_cancel->clicked.connect(slot(mainAppWin, &GLyxAppWin::remove_action));

      dialog_->destroy.connect(slot(this, &FormIndex::free));

      u_ = d_->updateBufferDependent.connect(slot(this, &FormIndex::updateSlot));
      h_ = d_->hideBufferDependent.connect(slot(this, &FormIndex::hide));

      updateSlot();  // make sure its up-to-date
    }
}

void FormIndex::updateSlot(bool switched)
{
  if (switched)
    {
      hide();
      return;
    }
  
  if (dialog_ != 0 &&
      lv_->view()->available())
    {
      keyword_->get_entry()->set_text(params.getContents().c_str());
      
      bool sens = (!(lv_->buffer()->isReadonly()));
      
      keyword_->set_sensitive(sens);
      b_ok->set_sensitive(sens);
    }
}
      
void FormIndex::hide()
{
  if (dialog_!=0) mainAppWin->remove_action();
}

void FormIndex::free()
{
  if (dialog_!=0)
    {
      dialog_ = 0;
      u_.disconnect();
      h_.disconnect();
      inset_ = 0;
      ih_.disconnect();
    }
}

void FormIndex::apply()
{
  if( lv_->buffer()->isReadonly() ) return;

  params.setContents( keyword_->get_entry()->get_text() );

  if( inset_ != 0 )
    {
      // Only update if contents have changed
      if( params != inset_->params() )
	{
	  inset_->setParams( params );
	  lv_->view()->updateInset( inset_, true );
	}
    }
  else
    {
      lv_->getLyXFunc()->Dispatch( LFUN_INDEX_INSERT,
				   params.getAsString() );
    }

  // save history
  keyword_->save_history();

  // hide the dialog
  hide();
}

