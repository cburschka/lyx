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

#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif


#include "gettext.h"
#include "Dialogs.h"
#include "FormCitation.h"
#include "LyXView.h"
#include "buffer.h"
#include "lyxfunc.h"
#include "support/filetools.h"
#include "support/LRegex.h"

#include <gtk--/scrolledwindow.h>
#include <gnome--/pixmap.h>
#include <gtk--/label.h>
#include <gtk--/box.h>
#include <gtk--/buttonbox.h>
#include <gnome--/entry.h>
#include <gnome--/stock.h>
#include <gtk--/separator.h>
#include <libgnome/gnome-config.h>
#include <gtk--/alignment.h>
#include "pixbutton.h"

// temporary solution for LyXView
#include "mainapp.h"
extern GLyxAppWin * mainAppWin;

using std::vector;
using std::pair;
using std::max;
using std::min;
using std::find;

#ifdef SIGC_CXX_NAMESPACES
using SigC::slot;
using SigC::bind;
#endif


// configuration keys
static string const  LOCAL_CONFIGURE_PREFIX("FormCitation");

static string const  CONF_PANE_INFO("paneinfo");
static string const  CONF_PANE_INFO_DEFAULT("=300");

static string const  CONF_COLUMN("column");
static string const  CONF_COLUMN_DEFAULT("=50");

static string const  CONF_REGEXP("regexp");
static string const  CONF_REGEXP_DEFAULT("=0");

static string const CONF_SEARCH("FormCitation_search");
static string const CONF_TEXTAFTER("FormCitation_textafter");

FormCitation::FormCitation(LyXView * lv, Dialogs * d)
  : lv_(lv), d_(d), inset_(0), u_(0), h_(0), ih_(0)
{
  // let the dialog be shown
  // These are permanent connections so we won't bother
  // storing a copy because we won't be disconnecting.
  d->showCitation.connect(slot(this, &FormCitation::showInset));
  d->createCitation.connect(slot(this, &FormCitation::createInset));

  cleanupWidgets();
}

FormCitation::~FormCitation()
{
  hide();
}

void FormCitation::showInset( InsetCommand * const inset )
{
  if( dialog_!=NULL || inset == 0 ) return;
  
  inset_ = inset;
  ih_ = inset_->hide.connect(slot(this, &FormCitation::hide));

  u_ = d_->updateBufferDependent.connect(slot(this, &FormCitation::update));
  h_ = d_->hideBufferDependent.connect(slot(this, &FormCitation::hide));
  
  params = inset->params();

  if ( params.getContents().empty() ) showStageSearch();
  else showStageAction();
}


void FormCitation::createInset( string const & arg )
{
  if( dialog_!=NULL ) return;
  
  u_ = d_->updateBufferDependent.connect(slot(this, &FormCitation::update));
  h_ = d_->hideBufferDependent.connect(slot(this, &FormCitation::hide));
  
  params.setFromString( arg );
  showStageSearch();
}


static
void parseBibTeX(string data,
		 string const & findkey,
		 string & keyvalue)
{
  unsigned int i;

  keyvalue = "";
  
  for (i=0; i<data.length(); ++i)
    if (data[i]=='\n' || data[i]=='\t')
      data[i] = ' ';
  
  data = frontStrip(data);
  while (!data.empty()
	 && data[0]!='='
	 && (data.find(' ')!=string::npos ||
	     data.find('=')!=string::npos) )
    {
      unsigned int keypos = min(data.find(' '), data.find('='));
      string key = lowercase( data.substr(0, keypos) );
      string value, tmp;
      char enclosing;
      
      data = data.substr(keypos, data.length()-1);
      data = frontStrip(strip(data));
      if (data.length() > 1 && data[0]=='=')
	{
	  data = frontStrip(data.substr(1, data.length()-1));
	  if (!data.empty())
	    {
	      keypos = 1;
	      if (data[0]=='{') enclosing = '}';
	      else if (data[0]=='"') enclosing = '"';
	      else { keypos=0; enclosing=' '; }

	      if (keypos &&
		  data.find(enclosing)!=string::npos &&
		  data.length()>1)
		{
		  tmp = data.substr(keypos, data.length()-1);
		  while (tmp.find('{')!=string::npos &&
			 tmp.find('}')!=string::npos &&
			 tmp.find('{') < tmp.find('}') &&
			 tmp.find('{') < tmp.find(enclosing))
		    {
		      keypos += tmp.find('{')+1;
		      tmp = data.substr(keypos, data.length()-1);
		      keypos += tmp.find('}')+1;
		      tmp = data.substr(keypos, data.length()-1);
		    }

		  if (tmp.find(enclosing)==string::npos) return;
		  else
		    {
		      keypos += tmp.find(enclosing);
		      tmp = data.substr(keypos, data.length()-1);
		    }

		  value = data.substr(1, keypos-1);

		  if (keypos+1<data.length()-1) data = frontStrip(data.substr(keypos+1, data.length()-1));
		  else data = "";
		}
	      else if (!keypos &&
		       (data.find(' ') ||
			data.find(','))
		       ) // numerical value ?
		{
		  keypos = data.length()-1;
		  if (data.find(' ')!=string::npos) keypos = data.find(' ');
		  if (data.find(',')!=string::npos &&
		      keypos > data.find(','))
		    keypos = data.find(',');

		  value = data.substr(0, keypos);
		  
		  if (keypos+1<data.length()-1) data = frontStrip(data.substr(keypos+1, data.length()-1));
		  else data = "";
		}
	      else return;

	      if (findkey == key) { keyvalue = value; return; } 

	      data = frontStrip(frontStrip(data,','));
	    }
	}
      else return;
    }
}


void FormCitation::cleanupWidgets()
{
  dialog_ = NULL;
  b_ok = NULL;
  b_cancel = NULL;
  search_text_ = NULL;
  info_ = NULL;
  text_after_ = NULL;
  button_unselect_ = NULL;
  button_up_ = NULL;
  button_down_ = NULL;
  button_regexp_ = NULL;
  clist_selected_ = NULL;
  clist_bib_ = NULL;
  paned_info_ = NULL;
}


void FormCitation::initWidgets()
{
  string const path = PACKAGE "/" + LOCAL_CONFIGURE_PREFIX;

  if (search_text_ != NULL)
    {
      search_text_->set_history_id(CONF_SEARCH);
      search_text_->set_max_saved(10);
      search_text_->load_history();
      search_text_->set_use_arrows_always(true);
    }

  if (text_after_ != NULL )
    {
      text_after_->set_history_id(CONF_TEXTAFTER);
      text_after_->set_max_saved(10);
      text_after_->load_history();
      text_after_->set_use_arrows_always(true);
      text_after_->get_entry()->set_text(params.getOptions());
    }

  if (button_regexp_ != NULL)
    {
      string w = path + "/" + CONF_REGEXP + CONF_REGEXP_DEFAULT;
      button_regexp_->set_active( (gnome_config_get_int(w.c_str()) > 0) );
    }

  if (paned_info_ != NULL)
    {
      string w = path + "/" + CONF_PANE_INFO + CONF_PANE_INFO_DEFAULT;
      paned_info_->set_position( gnome_config_get_int(w.c_str()) );
    }

  if (clist_bib_ != NULL)
    {
      // preferences
      clist_bib_->column(0).set_visiblity(false);
      clist_bib_->set_selection_mode(GTK_SELECTION_BROWSE);

      // setting up sizes of columns
      string w;
      int sz = clist_bib_->columns().size();
      for (int i = 0; i < sz; ++i)
	{
	  w = path + "/" + CONF_COLUMN + "_" + tostr(i) + CONF_COLUMN_DEFAULT;
	  clist_bib_->column(i).set_width( gnome_config_get_int(w.c_str()) );
	}
      
      // retrieving data
      vector<pair<string,string> > blist = lv_->buffer()->getBibkeyList();

      sz = blist.size();
      for (int i = 0; i < sz; ++i )
	{
	  bibkeys.push_back(blist[i].first);
	  bibkeysInfo.push_back(blist[i].second);
	}
      
      blist.clear();      
      
      // updating list
      search();

      if (clist_bib_->rows().size() > 0)
	{
	  clist_bib_->rows()[0].select();
	  selectionToggled(0, 0, NULL, true, false);
	}
    }

  if (clist_selected_ != NULL)
    {
      clist_selected_->set_selection_mode(GTK_SELECTION_BROWSE);

      // populating clist_selected_
      vector<string> r;
      string tmp, keys( params.getContents() );
      keys = frontStrip( split(keys, tmp, ',') );
      while( !tmp.empty() )
	{
	  r.clear();
	  r.push_back(tmp);
	  clist_selected_->rows().push_back(r);
	  
	  keys = frontStrip( split(keys, tmp, ',') );
	}

      if (clist_selected_->rows().size() > 0)
	{
	  clist_selected_->rows()[0].select();
	  selectionToggled(0, 0, NULL, true, true);
	}
    }

  updateButtons();
}


void FormCitation::storeWidgets()
{
  string const path = PACKAGE "/" + LOCAL_CONFIGURE_PREFIX;

  if (search_text_ != NULL) search_text_->save_history();

  if (text_after_ != NULL) text_after_->save_history();

  if (button_regexp_ != NULL)
    {
      string w = path + "/" + CONF_REGEXP;
      gnome_config_set_int(w.c_str(), button_regexp_->get_active());
    }

  if (paned_info_ != NULL) 
    {
      string w = path + "/" + CONF_PANE_INFO;
      gnome_config_set_int(w.c_str(), paned_info_->width() - info_->width());
    }

  if (clist_bib_ != NULL)
    {
      string w;
      int const sz = clist_bib_->columns().size();
      for (int i = 0; i < sz; ++i)
	{
	  w = path + "/" + CONF_COLUMN + "_" + tostr(i);
	  gnome_config_set_int(w.c_str(), clist_bib_->get_column_width(i));
	}
    }

  gnome_config_sync();
}


void FormCitation::showStageAction()
{
  if (!dialog_)
    {
      using namespace Gtk::Box_Helpers;

      Gtk::Alignment * mbox = manage( new Gtk::Alignment(0.5, 0.5, 0, 0) );
      Gtk::ButtonBox * bbox = manage( new Gtk::HButtonBox() );

      string const addlabel = N_("_Add new citation");
      string const editlabel = N_("_Edit/remove citation(s)");

      Gnome::PixButton * b_add  = manage(new Gnome::PixButton(addlabel, GNOME_STOCK_PIXMAP_NEW));
      Gnome::PixButton * b_edit = manage(new Gnome::PixButton(editlabel, GNOME_STOCK_PIXMAP_PROPERTIES));

      b_cancel = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL) ) );

      // set up spacing
      bbox->set_spacing(4);
      bbox->set_layout(GTK_BUTTONBOX_SPREAD);
      
      bbox->children().push_back(Element(*b_add, false, false));
      bbox->children().push_back(Element(*b_edit, false, false));
      bbox->children().push_back(Element(*b_cancel, false, false));

      mbox->add(*bbox);

      // accelerators
      Gtk::AccelGroup * accel = Gtk::AccelGroup::create();

      b_add->add_accelerator("clicked", *accel, b_add->get_accelkey(), 0, GTK_ACCEL_VISIBLE);
      b_edit->add_accelerator("clicked", *accel, b_edit->get_accelkey(), 0, GTK_ACCEL_VISIBLE);
      
      // packing dialog to main window
      dialog_ = mbox;
      mainAppWin->add_action(*dialog_, N_(" Citation: Select action "), false, accel);

      initWidgets();
      
      // setting focus
      gtk_widget_grab_focus (GTK_WIDGET(b_add->gtkobj()));

      // connecting signals
      b_add->clicked.connect(slot(this, &FormCitation::moveFromActionToSearch));
      b_edit->clicked.connect(slot(this, &FormCitation::moveFromActionToEdit));

      b_cancel->clicked.connect(slot(mainAppWin, &GLyxAppWin::remove_action));

      dialog_->destroy.connect(slot(this, &FormCitation::free));
    }
}


void FormCitation::moveFromActionToSearch()
{
  // stores configuration and cleans all widgets
  storeWidgets();
  cleanupWidgets();
  
  // moves to stage "search"
  mainAppWin->remove_action();
  showStageSearch();
}


void FormCitation::moveFromActionToEdit()
{
  // stores configuration and cleans all widgets
  storeWidgets();
  cleanupWidgets();
  
  // moves to stage "edit"
  mainAppWin->remove_action();
  showStageEdit();
}


void FormCitation::showStageSearch()
{
  if (!dialog_)
    {
      using namespace Gtk::Box_Helpers;

      Gtk::Box * mbox = manage( new Gtk::HBox() );
      Gtk::ButtonBox * bbox = manage( new Gtk::HButtonBox() );
      Gtk::Separator * sep = manage( new Gtk::VSeparator() );
      
      search_text_ = manage( new Gnome::Entry() );
      
      button_regexp_ = manage( new Gtk::CheckButton(N_("Use Regular Expression")) );

      b_ok = manage( new Gtk::Button(N_("Search")) );
      b_cancel = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL) ) );
      
      // set up spacing
      mbox->set_spacing(4);
      bbox->set_spacing(4);

      // packing
      bbox->children().push_back(Element(*b_ok, false, false));
      bbox->children().push_back(Element(*b_cancel, false, false));

      mbox->children().push_back(Element(*search_text_, true, true));
      mbox->children().push_back(Element(*button_regexp_, false, false));
      mbox->children().push_back(Element(*sep, false, false));
      mbox->children().push_back(Element(*bbox, false, false));

      // packing dialog to main window
      dialog_ = mbox;
      mainAppWin->add_action(*dialog_, N_(" Insert Citation: Enter keyword(s) or regular expression "));

      initWidgets();
      
      // setting focus
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(search_text_->get_entry()->gtkobj()), GTK_CAN_DEFAULT);
      gtk_widget_grab_focus (GTK_WIDGET(search_text_->get_entry()->gtkobj()));
      gtk_widget_grab_default (GTK_WIDGET(search_text_->get_entry()->gtkobj()));

      // connecting signals
      b_ok->clicked.connect(slot(this, &FormCitation::moveFromSearchToSelect));
      search_text_->get_entry()->activate.connect(slot(this, &FormCitation::moveFromSearchToSelect));

      b_cancel->clicked.connect(slot(mainAppWin, &GLyxAppWin::remove_action));
      dialog_->destroy.connect(slot(this, &FormCitation::free));
    }
}

void FormCitation::moveFromSearchToSelect()
{
  search_string_ = search_text_->get_entry()->get_text();
  use_regexp_ = button_regexp_->get_active();

  // stores configuration and cleans all widgets
  storeWidgets();
  cleanupWidgets();
  
  // moves to stage "select"
  mainAppWin->remove_action();
  showStageSelect();
}

void FormCitation::showStageSelect()
{
  if (!dialog_)
    {
      using namespace Gtk::Box_Helpers;

      Gtk::Box * mbox = manage( new Gtk::VBox() );
      Gtk::Box * tbox = manage( new Gtk::HBox() );
      Gtk::ButtonBox * bbox = manage( new Gtk::HButtonBox() );
      Gtk::Separator * sep = manage( new Gtk::HSeparator() );
      Gtk::ScrolledWindow * sw = manage( new Gtk::ScrolledWindow() );

      info_ = manage( new Gnome::Less() );
      paned_info_ = manage( new Gtk::HPaned() );
      text_after_ = manage( new Gnome::Entry() );
      
      b_ok = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_OK) ) );
      b_cancel = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL) ) );

      sw->set_policy(GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

      // constructing CList
      vector<string> colnames;
      colnames.push_back("INVISIBLE");
      colnames.push_back(N_("Key"));
      colnames.push_back(N_("Author(s)"));
      colnames.push_back(N_("Title"));
      colnames.push_back(N_("Year"));
      colnames.push_back(N_("Journal"));
      clist_bib_ = manage( new Gtk::CList(colnames) );

      bbox->set_layout(GTK_BUTTONBOX_END);
      
      // set up spacing
      mbox->set_spacing(4);
      bbox->set_spacing(4);
      tbox->set_spacing(4);

      // packing widgets
      sw->add(*clist_bib_);
      paned_info_->add1(*sw);
      paned_info_->add2(*info_);

      bbox->children().push_back(Element(*b_ok, false, false));
      bbox->children().push_back(Element(*b_cancel, false, false));

      tbox->children().push_back(Element(*manage(new Gtk::Label(N_("Text after"))), false, false));
      tbox->children().push_back(Element(*text_after_, true, true));
      tbox->children().push_back(Element(*manage(new Gtk::VSeparator()), false, false));
      tbox->children().push_back(Element(*bbox, false, false));

      mbox->children().push_back(Element(*paned_info_,true,true));
      mbox->children().push_back(Element(*sep, false, false));
      mbox->children().push_back(Element(*tbox, false, false));
      
      // packing dialog to main window
      dialog_ = mbox;
      mainAppWin->add_action(*dialog_, N_(" Insert Citation: Select citation "), true);

      initWidgets();
      
      // setting focus
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(b_ok->gtkobj()), GTK_CAN_DEFAULT);
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(b_cancel->gtkobj()), GTK_CAN_DEFAULT);
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(clist_bib_->gtkobj()), GTK_CAN_DEFAULT);
      gtk_widget_grab_focus (GTK_WIDGET(clist_bib_->gtkobj()));
      gtk_widget_grab_default (GTK_WIDGET(b_ok->gtkobj()));

      // connecting signals
      b_ok->clicked.connect(slot(this, &FormCitation::applySelect));
      text_after_->get_entry()->activate.connect(slot(this, &FormCitation::applySelect));

      b_cancel->clicked.connect(slot(mainAppWin, &GLyxAppWin::remove_action));

      dialog_->destroy.connect(slot(this, &FormCitation::free));

      clist_bib_->click_column.connect(slot(this, &FormCitation::sortBibList));
      clist_bib_->select_row.connect(bind(slot(this, &FormCitation::selectionToggled),
					  true, false));
      clist_bib_->unselect_row.connect(bind(slot(this, &FormCitation::selectionToggled),
					    false, false));
    }
}

void FormCitation::showStageEdit()
{
  if (!dialog_)
    {
      using namespace Gtk::Box_Helpers;

      Gtk::Box * mbox = manage( new Gtk::VBox() );
      Gtk::Box * tbox = manage( new Gtk::HBox() );
      Gtk::Box * t2box = manage( new Gtk::HBox() );
      Gtk::ButtonBox * bbox = manage( new Gtk::HButtonBox() );
      Gtk::ButtonBox * actbbox = manage( new Gtk::VButtonBox() );
      Gtk::ScrolledWindow * sw = manage( new Gtk::ScrolledWindow() );

      vector<string> colnames;
      colnames.push_back(" ");
      clist_selected_ = manage( new Gtk::CList(colnames) );
      clist_selected_->column_titles_hide();
      
      text_after_ = manage( new Gnome::Entry() );
      
      button_unselect_ = manage( new Gnome::PixButton( N_("_Remove"), GNOME_STOCK_PIXMAP_TRASH ) );
      button_up_ = manage( new Gnome::PixButton( N_("_Up"), GNOME_STOCK_PIXMAP_UP ) );
      button_down_ = manage( new Gnome::PixButton( N_("_Down"), GNOME_STOCK_PIXMAP_DOWN ) );

      b_ok = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_OK) ) );
      b_cancel = Gtk::wrap( GTK_BUTTON( gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL) ) );

      sw->set_policy(GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

      bbox->set_layout(GTK_BUTTONBOX_END);
      actbbox->set_layout(GTK_BUTTONBOX_START);
      
      // set up spacing
      mbox->set_spacing(4);
      bbox->set_spacing(4);
      actbbox->set_spacing(4);
      tbox->set_spacing(4);
      t2box->set_spacing(4);

      // packing widgets
      sw->add(*clist_selected_);

      bbox->children().push_back(Element(*b_ok, false, false));
      bbox->children().push_back(Element(*b_cancel, false, false));

      actbbox->children().push_back(Element(*button_unselect_, false, false));
      actbbox->children().push_back(Element(*button_up_, false, false));
      actbbox->children().push_back(Element(*button_down_, false, false));

      t2box->children().push_back(Element(*sw, true, true));
      t2box->children().push_back(Element(*actbbox, false, false));

      tbox->children().push_back(Element(*manage(new Gtk::Label(N_("Text after"))), false, false));
      tbox->children().push_back(Element(*text_after_, true, true));
      tbox->children().push_back(Element(*manage(new Gtk::VSeparator()), false, false));
      tbox->children().push_back(Element(*bbox, false, false));

      mbox->children().push_back(Element(*t2box,true,true));
      mbox->children().push_back(Element(*manage(new Gtk::HSeparator()), false, false));
      mbox->children().push_back(Element(*tbox, false, false));
      
      // accelerators
      Gtk::AccelGroup * accel = Gtk::AccelGroup::create();

      button_unselect_->add_accelerator("clicked", *accel, button_unselect_->get_accelkey(), 0, GTK_ACCEL_VISIBLE);
      button_up_->add_accelerator("clicked", *accel, button_up_->get_accelkey(), 0, GTK_ACCEL_VISIBLE);
      button_down_->add_accelerator("clicked", *accel, button_down_->get_accelkey(), 0, GTK_ACCEL_VISIBLE);

      // packing dialog to main window
      dialog_ = mbox;
      mainAppWin->add_action(*dialog_, N_(" Citation: Edit "), true, accel);

      initWidgets();

      // setting focus
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(b_ok->gtkobj()), GTK_CAN_DEFAULT);
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(b_cancel->gtkobj()), GTK_CAN_DEFAULT);
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(clist_selected_->gtkobj()), GTK_CAN_DEFAULT);
      gtk_widget_grab_focus (GTK_WIDGET(clist_selected_->gtkobj()));
      gtk_widget_grab_default (GTK_WIDGET(b_ok->gtkobj()));

      // connecting signals
      b_ok->clicked.connect(slot(this, &FormCitation::applyEdit));
      text_after_->get_entry()->activate.connect(slot(this, &FormCitation::applyEdit));

      b_cancel->clicked.connect(slot(mainAppWin, &GLyxAppWin::remove_action));

      dialog_->destroy.connect(slot(this, &FormCitation::free));

      button_unselect_->clicked.connect(slot(this, &FormCitation::removeCitation));
      button_up_->clicked.connect(slot(this, &FormCitation::moveCitationUp));
      button_down_->clicked.connect(slot(this, &FormCitation::moveCitationDown));      

      clist_selected_->select_row.connect(bind(slot(this, &FormCitation::selectionToggled),
					  true, true));
      clist_selected_->unselect_row.connect(bind(slot(this, &FormCitation::selectionToggled),
					    false, true));
    }
}


void FormCitation::addItemToBibList(int i)
{
  vector<string> r;
  string key, info;
  string val;

  key = bibkeys[i];
  info = bibkeysInfo[i];

  // don't change the order of these first two items:
  // callback functions depend on the data stored in the first column (its hided)
  // and in the second column (shown to user)
  r.push_back( tostr(i) ); 
  r.push_back( key );
  
  // this can be changed (configured by user?)
  parseBibTeX( info, "author", val);  r.push_back(val);
  parseBibTeX( info, "title", val);  r.push_back(val);
  parseBibTeX( info, "year", val);  r.push_back(val);
  parseBibTeX( info, "journal", val);  r.push_back(val);
  
  clist_bib_->rows().push_back(r);
}


void FormCitation::updateButtons()
{
  if (button_unselect_ != NULL) // => button_up_ and button_down_ are != NULL
    {
      bool sens;

      sens = (clist_selected_->selection().size()>0);
      button_unselect_->set_sensitive(sens);
      button_up_->set_sensitive(sens &&
				clist_selected_->selection()[0].get_row_num()>0);
      button_down_->set_sensitive(sens &&
				  clist_selected_->selection()[0].get_row_num() <
				  clist_selected_->rows().size()-1);
    }
}

void FormCitation::selectionToggled(gint            row,
				    gint            ,//column,
				    GdkEvent        * ,//event,
				    bool selected,
				    bool citeselected)
{
  if (!citeselected)
    {
      if (selected)
	{
	  bool keyfound = false;
	  string info;
	  
	  // the first column in clist_bib_ contains the index
	  keyfound = true;
	  info = bibkeysInfo[ strToInt(clist_bib_->cell(row,0).get_text()) ];

	  if (keyfound)
	    info_->show_string(info);
	  else
	    info_->show_string(N_("--- No such key in the database ---"));
	}
      else
	{
	  info_->show_string("");
	}
    }

  updateButtons();
}

void FormCitation::removeCitation()
{
  clist_selected_->rows().remove(clist_selected_->selection()[0]);
  updateButtons();
}

void FormCitation::moveCitationUp()
{
  int i = clist_selected_->selection()[0].get_row_num();
  clist_selected_->swap_rows( i-1, i );
  clist_selected_->row(i-1).select();
  updateButtons();
}

void FormCitation::moveCitationDown()
{
  int i = clist_selected_->selection()[0].get_row_num();
  clist_selected_->swap_rows( i+1, i );
  clist_selected_->row(i+1).select();
  updateButtons();
}

void FormCitation::hide()
{
  if (dialog_!=NULL) mainAppWin->remove_action();
}

void FormCitation::free()
{
  if (dialog_!=NULL)
    {
      // cleaning up
      cleanupWidgets();
      u_.disconnect();
      h_.disconnect();
      inset_ = 0;
      ih_.disconnect();
    }
}

void FormCitation::applySelect()
{
  if( lv_->buffer()->isReadonly() ) return;

  string contents;
  int sz;

  contents = frontStrip( strip(params.getContents()) );
  if (!contents.empty()) contents += ", ";
  
  sz = clist_bib_->selection().size();
  for (int i=0; i < sz; ++i)
    {
      if (i > 0) contents += ", ";
      contents += clist_bib_->selection()[i][1].get_text();
    }
  
  params.setContents( contents );
  params.setOptions( text_after_->get_entry()->get_text() );

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
      lv_->getLyXFunc()->Dispatch( LFUN_CITATION_INSERT,
				   params.getAsString() );
    }

  // close dialog
  storeWidgets();
  hide();
}

void FormCitation::applyEdit()
{
  if( lv_->buffer()->isReadonly() ) return;

  string contents;
  int const sz = clist_selected_->rows().size();
  for( int i = 0; i < sz; ++i )
    {
      if (i > 0) contents += ", ";
      contents += clist_selected_->cell(i, 0).get_text();
    }

  params.setContents( contents );
  params.setOptions( text_after_->get_entry()->get_text() );

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
      lv_->getLyXFunc()->Dispatch( LFUN_CITATION_INSERT,
				   params.getAsString() );
    }
  
  // close dialog
  storeWidgets();
  hide();
}

void FormCitation::sortBibList(gint col)
{
  clist_bib_->set_sort_column(col);
  clist_bib_->sort();
}

void FormCitation::search()
{
  if (use_regexp_) searchReg();
  else searchSimple();
}

// looking for entries which match regexp
void FormCitation::searchReg()
{
  string tmp, rexptxt( search_string_ );
  rexptxt = frontStrip( strip( rexptxt ) );
  
  LRegex reg(rexptxt);

  // populating clist_bib_
  clist_bib_->rows().clear();

  clist_bib_->freeze();

  int const sz = bibkeys.size();
  bool additem;
  for ( int i = 0; i < sz; ++i )
    {
      string data = bibkeys[i] + bibkeysInfo[i];

      if (rexptxt.empty()) additem = true;
      else additem = (reg.exec(data).size() > 0);
	     
      if ( additem ) addItemToBibList(i);
    }

  clist_bib_->sort();
  clist_bib_->thaw();
}

// looking for entries which contain all the words specified in search_text entry
void FormCitation::searchSimple()
{
  vector<string> searchwords;
  string tmp, stext( search_string_ );
  stext = frontStrip( strip( stext ) );
  stext = frontStrip( split(stext, tmp, ' ') );
  while( !tmp.empty() )
    {
      searchwords.push_back(tmp);
      stext = frontStrip( split(stext, tmp, ' ') );
    }
  
  // populating clist_bib_
  clist_bib_->rows().clear();

  clist_bib_->freeze();

  int const sz = bibkeys.size();
  bool additem;
  for ( int i = 0; i < sz; ++i )
    {
      string data = bibkeys[i] + bibkeysInfo[i];

      additem = true;

      int j, szs;
      for (j = 0, szs = searchwords.size();
	   additem && j < szs; ++j )
	if ( data.find(searchwords[j]) == string::npos )
	  additem = false;
	     
      if ( additem ) addItemToBibList(i);
    }

  clist_bib_->sort();
  clist_bib_->thaw();
}
