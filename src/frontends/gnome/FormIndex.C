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

extern "C" {
#include "diainsertindex_interface.h"
#include "support.h"
}

#include <gtk--/base.h>

#include "gettext.h"
#include "Dialogs.h"
#include "FormIndex.h"
#include "LyXView.h"
#include "buffer.h"
#include "form_index.h"
#include "lyxfunc.h"

FormIndex::FormIndex(LyXView * lv, Dialogs * d)
	: lv_(lv), d_(d), u_(0), h_(0), ih_(0), inset_(0), dialog_(NULL)
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
  if( dialog_!=NULL || inset == 0 ) return;
  
  inset_ = inset;
  ih_ = inset_->hide.connect(slot(this, &FormIndex::hide));
  
  params = inset->params();
  show();
}

void FormIndex::createInset( string const & arg )
{
  if( dialog_!=NULL ) return;
  
  params.setFromString( arg );
  show();
}

void FormIndex::show()
{
  if (!dialog_)
    {
      GtkWidget * pd = create_DiaInsertIndex();

      dialog_ = Gtk::wrap(pd);
      keyword_ = Gtk::wrap( GNOME_ENTRY( lookup_widget(pd, "keyword") ) );
      
      b_ok = Gtk::wrap( GTK_BUTTON( lookup_widget(pd, "button_ok") ) );
      b_cancel = Gtk::wrap( GTK_BUTTON( lookup_widget(pd, "button_cancel") ) );

      b_ok->clicked.connect(slot(this, &FormIndex::apply));
      b_ok->clicked.connect(dialog_->destroy.slot());
      b_cancel->clicked.connect(dialog_->destroy.slot());
      dialog_->destroy.connect(slot(this, &FormIndex::free));

      u_ = d_->updateBufferDependent.connect(slot(this, &FormIndex::update));
      h_ = d_->hideBufferDependent.connect(slot(this, &FormIndex::hide));

      if (!dialog_->is_visible()) dialog_->show_all();

      update();  // make sure its up-to-date
    }
  else
    {
      Gdk_Window dialog_win(dialog_->get_window());
      dialog_win.raise();
    }
}
      
void FormIndex::update()
{
  if (dialog_ != NULL &&
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
  if (dialog_!=NULL) dialog_->destroy();
}

void FormIndex::free()
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
				   params.getAsString().c_str() );
    }
}
