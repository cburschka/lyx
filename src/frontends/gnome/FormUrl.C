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
#include "FormUrl.h"
#include "LyXView.h"
#include "buffer.h"
#include "lyxfunc.h"

extern "C" {
#include "diainserturl_interface.h"
#include "support.h"
}

#include <gtk--/base.h>

FormUrl::FormUrl(LyXView * lv, Dialogs * d)
	: lv_(lv), d_(d), u_(0), h_(0), ih_(0), inset_(0), dialog_(NULL)
{
  // let the dialog be shown
  // These are permanent connections so we won't bother
  // storing a copy because we won't be disconnecting.
  d->showUrl.connect(slot(this, &FormUrl::showInset));
  d->createUrl.connect(slot(this, &FormUrl::createInset));
}


FormUrl::~FormUrl()
{
  hide();
}

void FormUrl::showInset( InsetCommand * const inset )
{
  if( dialog_!=NULL || inset == 0 ) return;
  
  inset_ = inset;
  ih_ = inset_->hide.connect(slot(this, &FormUrl::hide));
  
  params = inset->params();
  show();
}


void FormUrl::createInset( string const & arg )
{
  if( dialog_!=NULL ) return;
  
  params.setFromString( arg );
  show();
}

void FormUrl::show()
{
  if (!dialog_)
    {
      GtkWidget * pd = create_DiaInsertUrl();

      dialog_ = Gtk::wrap(pd);
      url_ = Gtk::wrap( GNOME_ENTRY( lookup_widget(pd, "url") ) );
      name_ = Gtk::wrap( GNOME_ENTRY( lookup_widget(pd, "name") ) );
      html_type_ = Gtk::wrap( GTK_CHECK_BUTTON( lookup_widget(pd, "html_type") ) );
      
      b_ok = Gtk::wrap( GTK_BUTTON( lookup_widget(pd, "button_ok") ) );
      b_cancel = Gtk::wrap( GTK_BUTTON( lookup_widget(pd, "button_cancel") ) );

      b_ok->clicked.connect(slot(this, &FormUrl::apply));
      b_ok->clicked.connect(dialog_->destroy.slot());
      b_cancel->clicked.connect(dialog_->destroy.slot());
      dialog_->destroy.connect(slot(this, &FormUrl::free));

      u_ = d_->updateBufferDependent.connect(slot(this, &FormUrl::update));
      h_ = d_->hideBufferDependent.connect(slot(this, &FormUrl::hide));

      if (!dialog_->is_visible()) dialog_->show_all();

      update();  // make sure its up-to-date
    }
  else
    {
      Gdk_Window dialog_win(dialog_->get_window());
      dialog_win.raise();
    }
}

void FormUrl::update()
{
  if (dialog_ != NULL &&
      lv_->view()->available())
    {
      url_->get_entry()->set_text(params.getContents().c_str());
      name_->get_entry()->set_text(params.getOptions().c_str());

      html_type_->set_active( (params.getCmdName() == "htmlurl") );

      bool sens = (!(lv_->buffer()->isReadonly()));

      html_type_->set_sensitive(sens);
      url_->set_sensitive(sens);
      name_->set_sensitive(sens);
      b_ok->set_sensitive(sens);
    }
}

void FormUrl::hide()
{
  if (dialog_!=NULL) dialog_->destroy();
}

void FormUrl::free()
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

void FormUrl::apply()
{
  if( lv_->buffer()->isReadonly() ) return;

  params.setContents( url_->get_entry()->get_text() );
  params.setOptions( name_->get_entry()->get_text() );

  if (html_type_->get_active())
    params.setCmdName("htmlurl");
  else
    params.setCmdName("url");
  
  if( inset_ != 0 )
    {
      // Only update if contents have changed
      if( params != inset_->params() ) {
	inset_->setParams( params );
	lv_->view()->updateInset( inset_, true );
      }
    }
  else
    {
      lv_->getLyXFunc()->Dispatch( LFUN_INSERT_URL,
				   params.getAsString().c_str() );
    }
}
