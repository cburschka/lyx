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
#include <vector>

#ifdef __GNUG__
#pragma implementation
#endif


#include "gettext.h"
#include "Dialogs.h"
#include "FormToc.h"
#include "LyXView.h"
#include "form_toc.h"
#include "lyxtext.h"

extern "C" {
#include "diatoc_interface.h"
#include "support.h"
}

#include <gtk--/base.h>
#include <gtk--/button.h>
#include <gtk--/label.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/menu.h>
#include <gtk--/menuitem.h>

using SigC::bind;

FormToc::FormToc(LyXView * lv, Dialogs * d)
  : lv_(lv), d_(d), inset_(0), u_(0), h_(0), ih_(0), dialog_(NULL), ignore_callback_(false)
{
  // let the dialog be shown
  // These are permanent connections so we won't bother
  // storing a copy because we won't be disconnecting.
  d->showTOC.connect(slot(this, &FormToc::showInset));
  d->createTOC.connect(slot(this, &FormToc::createInset));
}


FormToc::~FormToc()
{
  hide();
}

void FormToc::showInset( InsetCommand * const inset )
{
  if( dialog_!=NULL || inset == 0 ) return;
  
  inset_ = inset;
  ih_ = inset_->hide.connect(slot(this, &FormToc::hide));
  
  params = inset->params();
  show();
}


void FormToc::createInset( string const & arg )
{
  if( dialog_!=NULL ) return;
  
  params.setFromString( arg );
  show();
}


void FormToc::show()
{
  Gtk::Button * b_refresh;
  Gtk::Button * b_close;
  Gtk::ScrolledWindow *scrolled_window;

  if (!dialog_)
    {
      GtkWidget * pd = create_DiaToc();

      dialog_ = Gtk::wrap( GNOME_DIALOG(pd) );
      choice_ = Gtk::wrap( GTK_OPTION_MENU( lookup_widget(pd, "choice") ) );
      scrolled_window = Gtk::wrap( GTK_SCROLLED_WINDOW( lookup_widget(pd, "scrolledwindow") ) );

      list_ = manage( new Gtk::List() );
      scrolled_window->add_with_viewport(*list_);

      // fill choice
      Gtk::MenuItem * e;

      choice_->get_menu()->items().clear();
      
      e = manage( new Gtk::MenuItem(N_("Table of Contents")) );
      e->activate.connect(bind<Buffer::TocType>(slot(this, &FormToc::changeList), Buffer::TOC_TOC));
      choice_->get_menu()->append( *e );

      e = manage( new Gtk::MenuItem(N_("List of Figures")) );
      e->activate.connect(bind<Buffer::TocType>(slot(this, &FormToc::changeList), Buffer::TOC_LOF));
      choice_->get_menu()->append( *e );

      e = manage( new Gtk::MenuItem(N_("List of Tables")) );
      e->activate.connect(bind<Buffer::TocType>(slot(this, &FormToc::changeList), Buffer::TOC_LOT));
      choice_->get_menu()->append( *e );

      e = manage( new Gtk::MenuItem(N_("List of Algorithms")) );
      e->activate.connect(bind<Buffer::TocType>(slot(this, &FormToc::changeList), Buffer::TOC_LOA));
      choice_->get_menu()->append( *e );

      // wrap buttons and connect slots
      b_refresh = Gtk::wrap( GTK_BUTTON( lookup_widget(pd, "button_refresh") ) );
      b_close   = Gtk::wrap( GTK_BUTTON( lookup_widget(pd, "button_close") ) );
      
      b_refresh->clicked.connect(slot(this, &FormToc::update));
      b_close->clicked.connect(dialog_->destroy.slot());
      dialog_->destroy.connect(slot(this, &FormToc::free));

      u_ = d_->updateBufferDependent.connect(slot(this, &FormToc::update));
      h_ = d_->hideBufferDependent.connect(slot(this, &FormToc::hide));

      if (!dialog_->is_visible()) dialog_->show_all();

      update();  // make sure its up-to-date
    }
  else
    {
      Gdk_Window dialog_win(dialog_->get_window());
      dialog_win.raise();
    }
}


void FormToc::update()
{
  Buffer::TocType type;
  string wintitle;

  if (dialog_ != NULL &&
      !lv_->view()->available())
    {
      wintitle = N_( "*** No Document ***");
      dialog_->set_title(wintitle);
      list_->items().clear();

      Gtk::ListItem * l = manage( new Gtk::ListItem(wintitle) );
      list_->items().push_back( *l );
      return;
    }
  
  
  if (dialog_ != NULL &&
      lv_->view()->available())
    {

      int selection = 0;
      
      if( params.getCmdName() == "tableofcontents" )
	{
	  type = Buffer::TOC_TOC;
	  wintitle = N_("Table of Contents");
	  selection = 0;
	}
      else if( params.getCmdName() == "listoffigures" )
	{
	  type = Buffer::TOC_LOF;
	  wintitle = N_("List of Figures");
	  selection = 1;
	}
      else if( params.getCmdName() == "listofalgorithms" )
	{
	  type = Buffer::TOC_LOA;
	  wintitle = N_("List of Algorithms");
	  selection = 3;
	}
      else
	{
	  type = Buffer::TOC_LOT;
	  wintitle = N_("List of Tables");
	  selection = 2;
	}

      ignore_callback_ = true;
      choice_->set_history(selection);
      ignore_callback_ = false;
      
      list_->items().clear();

      dialog_->set_title(wintitle);

      vector<Buffer::TocItem> toclist = (lv_->view()->buffer()->getTocList())[type];

      Gtk::ListItem * item;

      vector<Buffer::TocItem>::const_iterator end = toclist.end();
      for (vector<Buffer::TocItem>::const_iterator it = toclist.begin();
	   it != end; ++it)
	{
	  item = manage( new Gtk::ListItem(string(4*(*it).depth,' ')+(*it).str) );
	  item->select.connect(bind<Buffer::TocItem>(slot(this,&FormToc::apply), (*it)));
	  list_->add( *item );
	}
    }

  dialog_->show_all();
}

void FormToc::apply(Buffer::TocItem tg)
{
  if (!lv_->view()->available()) return;
  
  lv_->view()->beforeChange();
  lv_->view()->text->SetCursor( lv_->view(), tg.par, 0 );
  lv_->view()->text->sel_cursor = lv_->view()->text->cursor;
  lv_->view()->update(BufferView::SELECT|BufferView::FITCUR);
}

void FormToc::changeList(Buffer::TocType type)
{
  if (!ignore_callback_)
    {
      switch (type) {
      case Buffer::TOC_TOC :
	{
	  params.setCmdName("tableofcontents");
	  break;
	}
      case Buffer::TOC_LOF :
	{
	  params.setCmdName("listoffigures");
	  break;
	}
      case Buffer::TOC_LOT :
	{
	  params.setCmdName("listoftabels");
	  break;
	}
      case Buffer::TOC_LOA :
	{
	  params.setCmdName("listofalgorithms");
	  break;
	}
      };
      update();
    }
}

void FormToc::hide()
{
  if (dialog_!=NULL) dialog_->destroy();
}

void FormToc::free()
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
 
