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

#include <gtk--/label.h>
#include <gtk--/table.h>
#include <gtk--/box.h>
#include <gtk--/buttonbox.h>
#include <gtk--/base.h>
#include <gtk--/separator.h>

// temporary solution for LyXView
#include "mainapp.h"
extern GLyxAppWin * mainAppWin;

// configuration keys
static string const CONF_ENTRY_URL("FormUrl_url");
static string const CONF_ENTRY_NAME("FormUrl_name");

FormUrl::FormUrl(LyXView * lv, Dialogs * d)
	: lv_(lv), d_(d), inset_(0), u_(0), h_(0), ih_(0), dialog_(0)
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
  if( dialog_!=0 || inset == 0 ) return;
  
  inset_ = inset;
  ih_ = inset_->hideDialog.connect(slot(this, &FormUrl::hide));
  
  params = inset->params();
  show();
}


void FormUrl::createInset( string const & arg )
{
  if( dialog_!=0 ) return;
  
  params.setFromString( arg );
  show();
}

void FormUrl::show()
{
  if (!dialog_)
    {
      using namespace Gtk::Box_Helpers;

      Gtk::Label * label;
      Gtk::Table * table = manage( new Gtk::Table(2, 2, FALSE) );
      Gtk::Box * mbox = manage( new Gtk::HBox() );
      Gtk::ButtonBox * bbox = manage( new Gtk::VButtonBox() );
      Gtk::Separator * sep = manage( new Gtk::VSeparator() );

      url_ = manage( new Gnome::Entry() );
      name_ = manage( new Gnome::Entry() );
      html_type_ = manage( new Gtk::CheckButton(_("HTML type")) );
      
      b_ok = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_OK) ) );
      b_cancel = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL) ) );
      
      // set up spacing
      table->set_row_spacings(2);
      table->set_col_spacings(2);
      mbox->set_spacing(2);
      bbox->set_spacing(4);

      // configure entries
      url_->set_history_id(CONF_ENTRY_URL);
      url_->set_max_saved(10);
      url_->load_history();
      url_->set_use_arrows_always(true);
      
      name_->set_history_id(CONF_ENTRY_NAME);
      name_->set_max_saved(10);
      name_->load_history();
      name_->set_use_arrows_always(true);
      
      // pack widgets
      bbox->children().push_back(Element(*b_ok, false, false));
      bbox->children().push_back(Element(*b_cancel, false, false));
      
      label = manage( new Gtk::Label(_("URL")) );
      table->attach( *label, 0, 1, 0, 1, 0, 0 );
      label = manage( new Gtk::Label(_("Name")) );
      table->attach( *label, 0, 1, 1, 2, 0, 0 );
      table->attach( *url_, 1, 2, 0, 1 );
      table->attach( *name_, 1, 2, 1, 2 );

      mbox->children().push_back(Element(*table));
      mbox->children().push_back(Element(*html_type_, false, false));
      mbox->children().push_back(Element(*sep, false, false));
      mbox->children().push_back(Element(*bbox, false, false));
      
      // packing dialog to main window
      dialog_ = mbox;
      mainAppWin->add_action(*dialog_, _(" URL "));

      // setting focus
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(url_->get_entry()->gtkobj()), GTK_CAN_DEFAULT);
      gtk_widget_grab_focus (GTK_WIDGET(url_->get_entry()->gtkobj()));
      gtk_widget_grab_default (GTK_WIDGET(url_->get_entry()->gtkobj()));

      // connecting signals
      b_ok->clicked.connect(slot(this, &FormUrl::apply));
      name_->get_entry()->activate.connect(slot(this, &FormUrl::apply));

      b_cancel->clicked.connect(slot(mainAppWin, &GLyxAppWin::remove_action));

      dialog_->destroy.connect(slot(this, &FormUrl::free));

      u_ = d_->updateBufferDependent.connect(slot(this, &FormUrl::updateSlot));
      h_ = d_->hideBufferDependent.connect(slot(this, &FormUrl::hide));

      updateSlot();  // make sure its up-to-date
    }
}

void FormUrl::updateSlot(bool switched)
{
  if (switched)
    {
      hide();
      return;
    }
  
  if (dialog_ != 0 &&
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
  if (dialog_!=0) mainAppWin->remove_action();
}

void FormUrl::free()
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
				   params.getAsString() );
    }

  // save history
  url_->save_history();
  name_->save_history();

  // hide the dialog
  hide();
}
