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
#include "FormRef.h"
#include "LyXView.h"
#include "buffer.h"
#include "lyxfunc.h"

#include <gtk--/box.h>
#include <gtk--/buttonbox.h>
#include <gtk--/alignment.h>
#include <gtk--/separator.h>
#include <gtk--/label.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/entry.h>
#include <gtk--/table.h>

#include <gtk--/menu.h>
#include <gtk--/menuitem.h>

// temporary solution for LyXView
#include "mainapp.h"
extern GLyxAppWin * mainAppWin;

using SigC::bind;

// configuration keys
static string const CONF_ENTRY_NAME("FormRef_name");

// goto button labels
static string const GOTO_REF_LABEL(N_("Goto reference"));
static string const GOTO_BACK_LABEL(N_("Go back"));

FormRef::FormRef(LyXView * lv, Dialogs * d)
	: lv_(lv), d_(d), inset_(0), u_(0), h_(0), ih_(0), dialog_(0)
{
  // let the dialog be shown
  // These are permanent connections so we won't bother
  // storing a copy because we won't be disconnecting.
  d->showRef.connect(slot(this, &FormRef::showInset));
  d->createRef.connect(slot(this, &FormRef::createInset));
}


FormRef::~FormRef()
{
  hide();
}

void FormRef::showInset( InsetCommand * const inset )
{
  if( dialog_!=0 || inset == 0 ) return;
  
  inset_ = inset;
  ih_ = inset_->hide.connect(slot(this, &FormRef::hide));

  acttype_ = EDIT;
  
  params = inset->params();
  showStageAction();
}


void FormRef::createInset( string const & arg )
{
  if( dialog_!=0 ) return;
  
  acttype_ = INSERT;

  params.setFromString( arg );

  refs = lv_->buffer()->getLabelList();

  if (refs.empty()) showStageError(_("*** No labels found in document ***"));
  else showStageSelect();
}

void FormRef::showStageError(string const & mess)
{
  if (!dialog_)
    {
      using namespace Gtk::Box_Helpers;

      Gtk::Alignment * alig = manage( new Gtk::Alignment(0.5, 0.5, 0, 0) );
      Gtk::Box * box = manage( new Gtk::HBox() );

      b_cancel = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL) ) );

      box->set_spacing(4);

      box->children().push_back(Element(*manage( new Gtk::Label(mess) ), false, false));
      box->children().push_back(Element(*manage(new Gtk::VSeparator()), false, false));
      box->children().push_back(Element(*b_cancel, false, false));

      alig->add(*box);

      // packing dialog to main window
      dialog_ = alig;
      mainAppWin->add_action(*dialog_, _(" Reference "));

      // setting focus
      gtk_widget_grab_focus (GTK_WIDGET(b_cancel->gtkobj()));

      // connecting signals
      b_cancel->clicked.connect(slot(mainAppWin, &GLyxAppWin::remove_action));
      dialog_->destroy.connect(slot(this, &FormRef::free));

      u_ = d_->updateBufferDependent.connect(slot(this, &FormRef::updateSlot));
      h_ = d_->hideBufferDependent.connect(slot(this, &FormRef::hide));
    }
}
      

void FormRef::showStageSelect()
{
  if (!dialog_)
    {
      using namespace Gtk::Box_Helpers;

      Gtk::Box * mbox = manage( new Gtk::HBox() );
      Gtk::ButtonBox * bbox = manage( new Gtk::VButtonBox() );
      Gtk::Alignment * alig = manage( new Gtk::Alignment(0.5, 0.5, 0, 0) );
      Gtk::ScrolledWindow * sw = manage( new Gtk::ScrolledWindow() );

      // constructing CList
      vector<string> colnames;
      colnames.push_back("INVISIBLE");
      list_ = manage( new Gtk::CList(colnames) );

      // populating CList
      vector<string> r;
      vector<string>::const_iterator end = refs.end();
      for (vector<string>::const_iterator it = refs.begin(); it != end; ++it)
	{
	  r.clear();
	  r.push_back(*(it));
	  list_->rows().push_back(r);
	}
      list_->rows()[0].select(); // there is always at least one item. otherwise we go to stateError
      
      b_ok = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_OK) ) );
      b_cancel = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL) ) );

      // policy
      list_->set_selection_mode(GTK_SELECTION_BROWSE); // this ensures that we have always item selected
      list_->column_titles_hide();

      sw->set_policy(GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

      // set up spacing
      mbox->set_spacing(2);
      bbox->set_spacing(4);

      // pack widgets
      sw->add(*list_);

      bbox->children().push_back(Element(*b_ok, false, false));
      bbox->children().push_back(Element(*b_cancel, false, false));

      alig->add( *bbox );
      
      mbox->children().push_back(Element(*sw, true, true));
      mbox->children().push_back(Element(*manage(new Gtk::VSeparator()), false, false));
      mbox->children().push_back(Element(*alig, false, false));
      
      // packing dialog to main window
      dialog_ = mbox;
      mainAppWin->add_action(*dialog_, _(" Reference: Select reference "), true);

      // setting focus
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(b_ok->gtkobj()), GTK_CAN_DEFAULT);
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(b_cancel->gtkobj()), GTK_CAN_DEFAULT);
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(list_->gtkobj()), GTK_CAN_DEFAULT);
      gtk_widget_grab_focus (GTK_WIDGET(list_->gtkobj()));
      gtk_widget_grab_default (GTK_WIDGET(b_ok->gtkobj()));

      // connecting signals
      b_ok->clicked.connect(slot(this, &FormRef::moveFromSelectToAction));
      b_cancel->clicked.connect(slot(mainAppWin, &GLyxAppWin::remove_action));

      dialog_->destroy.connect(slot(this, &FormRef::free));

      u_ = d_->updateBufferDependent.connect(slot(this, &FormRef::updateSlot));
      h_ = d_->hideBufferDependent.connect(slot(this, &FormRef::hide));
    }
}

void FormRef::moveFromSelectToAction()
{
  params.setContents( list_->selection()[0][0].get_text() );

  // moves to stage "search"
  mainAppWin->remove_action();
  showStageAction();
}

void FormRef::showStageAction()
{
  if (!dialog_)
    {
      using namespace Gtk::Box_Helpers;

      Gtk::Table * table = manage( new Gtk::Table(2, 2, FALSE) );
      Gtk::Box * mbox = manage( new Gtk::HBox() );
      Gtk::ButtonBox * bbox = manage( new Gtk::HButtonBox() );

      b_goto = manage(new Gnome::PixButton(GOTO_REF_LABEL, GNOME_STOCK_PIXMAP_JUMP_TO));
      gototype_ = GOREF;
      
      name_ = manage( new Gnome::Entry() );

      choice_ = manage( new Gtk::OptionMenu() );
      
      b_ok = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_OK) ) );
      b_cancel = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL) ) );
      
      // set up spacing
      table->set_row_spacings(4);
      table->set_col_spacings(4);

      mbox->set_spacing(4);

      bbox->set_spacing(4);
      bbox->set_layout(GTK_BUTTONBOX_SPREAD);

      // configure entries
      name_->set_history_id(CONF_ENTRY_NAME);
      name_->set_max_saved(10);
      name_->load_history();
      name_->set_use_arrows_always(true);
      if( lv_->buffer()->isLatex() ) name_->set_sensitive(false); // Name is irrelevant to LaTeX documents

      // fill choice
      Gtk::Menu * menu = manage( new Gtk::Menu() );
      Gtk::MenuItem * e;
      
      e = manage( new Gtk::MenuItem(_("Ref")) );
      e->activate.connect(bind<Type>(slot(this, &FormRef::changeType), REF));
      e->show();
      menu->append( *e );

      e = manage( new Gtk::MenuItem(_("Page")) );
      e->activate.connect(bind<Type>(slot(this, &FormRef::changeType), PAGEREF));
      e->show();
      menu->append( *e );

      e = manage( new Gtk::MenuItem(_("TextRef")) );
      e->activate.connect(bind<Type>(slot(this, &FormRef::changeType), VREF));
      e->show();
      menu->append( *e );

      e = manage( new Gtk::MenuItem(_("TextPage")) );
      e->activate.connect(bind<Type>(slot(this, &FormRef::changeType), VPAGEREF));
      e->show();
      menu->append( *e );

      e = manage( new Gtk::MenuItem(_("PrettyRef")) );
      e->activate.connect(bind<Type>(slot(this, &FormRef::changeType), PRETTYREF));
      e->show();
      menu->append( *e );

      choice_-> set_menu ( *menu );

      switch ( getType() ) {
      case REF: 	{ choice_-> set_history(0); break; }
      case PAGEREF:	{ choice_-> set_history(1); break; }
      case VREF:	{ choice_-> set_history(2); break; }
      case VPAGEREF:	{ choice_-> set_history(3); break; }
      case PRETTYREF:	{ choice_-> set_history(4); break; }
      }

      changeType( getType() );
      
      // filling widgets with data
      name_->get_entry()->set_text(params.getOptions());
      

      // pack widgets
      bbox->children().push_back(Element(*b_goto, false, false));
      bbox->children().push_back(Element(*b_ok, false, false));
      bbox->children().push_back(Element(*b_cancel, false, false));
      
      table->attach( *manage( new Gtk::Label(_("Type:")) ), 0, 1, 0, 1, 0, 0 );
      table->attach( *manage( new Gtk::Label(_("Name:")) ), 0, 1, 1, 2, 0, 0 );
      table->attach( *choice_, 1, 2, 0, 1 );
      table->attach( *name_, 1, 2, 1, 2 );

      mbox->children().push_back(Element(*table, true, true));
      mbox->children().push_back(Element(*manage( new Gtk::VSeparator() ), false, false ));
      mbox->children().push_back(Element(*bbox, false, false));

      // packing dialog to main window
      dialog_ = mbox;
      mainAppWin->add_action(*dialog_, string(_(" Reference: ")) + params.getContents() + string(" "));

      // setting focus
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(b_cancel->gtkobj()), GTK_CAN_DEFAULT);
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(b_ok->gtkobj()), GTK_CAN_DEFAULT);
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(b_goto->gtkobj()), GTK_CAN_DEFAULT);
      gtk_widget_grab_default (GTK_WIDGET(b_ok->gtkobj()));
      gtk_widget_grab_focus (GTK_WIDGET(choice_->gtkobj()));

      // connecting signals
      b_cancel->clicked.connect(slot(mainAppWin, &GLyxAppWin::remove_action));
      b_ok->clicked.connect(slot(this, &FormRef::apply));
      b_goto->clicked.connect(slot(this, &FormRef::gotoRef));

      dialog_->destroy.connect(slot(this, &FormRef::free));

      u_ = d_->updateBufferDependent.connect(slot(this, &FormRef::updateSlot));
      h_ = d_->hideBufferDependent.connect(slot(this, &FormRef::hide));
    }
}

void FormRef::updateSlot(bool buffchanged)
{
  if (buffchanged) hide();
}

void FormRef::hide()
{
  if (dialog_!=0) mainAppWin->remove_action();
}

void FormRef::free()
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

void FormRef::gotoRef()
{
  switch (gototype_) {
  case GOREF:
    {
      lv_->getLyXFunc()-> Dispatch(LFUN_REF_GOTO, params.getContents());

      gototype_ = GOBACK;
      b_goto->set_text(GOTO_BACK_LABEL);
      break;
    }
  case GOBACK:
    {
      lv_->getLyXFunc()->Dispatch(LFUN_BOOKMARK_GOTO, "0");

      gototype_ = GOREF;
      b_goto->set_text(GOTO_REF_LABEL);
      break;
    }
  }
}

void FormRef::apply()
{
  if ( lv_->buffer()->isReadonly() )
    return;
  
  params.setCmdName(getName(reftype_));
  params.setOptions(name_->get_entry()->get_text());
  
  if (inset_ != 0)
    {
      // Only update if contents have changed
      if (params != inset_->params())
	{
	  inset_->setParams(params);
	  lv_->view()->updateInset(inset_, true);
	}
    }
  else
    {
      lv_->getLyXFunc()->Dispatch(LFUN_REF_INSERT,
				  params.getAsString());
      lv_->getLyXFunc()->Dispatch(LFUN_BOOKMARK_SAVE, "0");
    }

  // save configuration
  name_->save_history();
  
  // hide the dialog
  hide();
}

void FormRef::changeType(Type t)
{
  reftype_ = t;
}

FormRef::Type FormRef::getType() const
{
  Type type;
  
  if( params.getCmdName() == "ref" )
    type = REF;
  
  else if( params.getCmdName() == "pageref" )
    type = PAGEREF;
  
  else if( params.getCmdName() == "vref" )
    type = VREF;
  
  else if( params.getCmdName() == "vpageref" )
    type = VPAGEREF;
  
  else
    type = PRETTYREF;
  
  return type;
}


string FormRef::getName( Type type ) const
{
  string name;
  
  switch( type ) {
  case REF:
    name = "ref";
    break;
  case PAGEREF:
    name = "pageref";
    break;
  case VREF:
    name = "vref";
    break;
  case VPAGEREF:
    name = "vpageref";
    break;
  case PRETTYREF:
    name = "prettyref";
    break;
  }
  
  return name;
}
