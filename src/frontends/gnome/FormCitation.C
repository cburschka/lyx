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

static string const  CONF_DIALOG_WIDTH("width");
static string const  CONF_DIALOG_WIDTH_DEFAULT("=550");

static string const  CONF_DIALOG_HEIGTH("heigth");
static string const  CONF_DIALOG_HEIGTH_DEFAULT("=550");

static string const  CONF_PANE_INFO("paneinfo");
static string const  CONF_PANE_INFO_DEFAULT("=300");

static string const  CONF_PANE_KEY("panekey");
static string const  CONF_PANE_KEY_DEFAULT("=225");

static string const  CONF_COLUMN("column");
static string const  CONF_COLUMN_DEFAULT("=50");

static string const  CONF_REGEXP("regexp");
static string const  CONF_REGEXP_DEFAULT("=0");

static string const CONF_SEARCH("FormCitation_search");
static string const CONF_TEXTAFTER("FormCitation_textafter");

FormCitation::FormCitation(LyXView * lv, Dialogs * d)
	: lv_(lv), d_(d), u_(0), h_(0), ih_(0), inset_(0), dialog_(NULL)
{
  // let the dialog be shown
  // These are permanent connections so we won't bother
  // storing a copy because we won't be disconnecting.
  d->showCitation.connect(slot(this, &FormCitation::showInset));
  d->createCitation.connect(slot(this, &FormCitation::createInset));
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
  //showStageEdit();
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

      search_text_->set_history_id(CONF_SEARCH);
      search_text_->set_max_saved(10);
      search_text_->load_history();
      search_text_->set_use_arrows_always(true);
      
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

      // setting focus
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(search_text_->get_entry()->gtkobj()), GTK_CAN_DEFAULT);
      gtk_widget_grab_focus (GTK_WIDGET(search_text_->get_entry()->gtkobj()));
      gtk_widget_grab_default (GTK_WIDGET(search_text_->get_entry()->gtkobj()));

      // restoring regexp setting
      string path = PACKAGE "/" + LOCAL_CONFIGURE_PREFIX;
      string w;
      w = path + "/" + CONF_REGEXP + CONF_REGEXP_DEFAULT;
      button_regexp_->set_active( (gnome_config_get_int(w.c_str()) > 0) );

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
  
  // saving configuration
  search_text_->save_history();

  string path = PACKAGE "/" + LOCAL_CONFIGURE_PREFIX;
  string w;
  w = path + "/" + CONF_REGEXP;
  gnome_config_set_int(w.c_str(), button_regexp_->get_active());
  gnome_config_sync();
  
  // moves to stage "select"
  dialog_ = NULL;
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

      // setup text_after_
      text_after_->set_history_id(CONF_TEXTAFTER);
      text_after_->set_max_saved(10);
      text_after_->load_history();
      text_after_->set_use_arrows_always(true);

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
      clist_bib_->column(0).set_visiblity(false);

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

      // setting focus
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(b_ok->gtkobj()), GTK_CAN_DEFAULT);
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(b_cancel->gtkobj()), GTK_CAN_DEFAULT);
      GTK_WIDGET_SET_FLAGS (GTK_WIDGET(clist_bib_->gtkobj()), GTK_CAN_DEFAULT);
      gtk_widget_grab_focus (GTK_WIDGET(clist_bib_->gtkobj()));
      gtk_widget_grab_default (GTK_WIDGET(b_ok->gtkobj()));

      // setting sizes of the widgets
      string path;
      string w, h;
      path = PACKAGE "/" + LOCAL_CONFIGURE_PREFIX;

      w = path + "/" + CONF_PANE_INFO + CONF_PANE_INFO_DEFAULT;
      paned_info_->set_position( gnome_config_get_int(w.c_str()) );

      int i, sz;
      for (i = 0, sz = clist_bib_->columns().size(); i < sz; ++i)
	{
	  w = path + "/" + CONF_COLUMN + "_" + tostr(i) + CONF_COLUMN_DEFAULT;
	  clist_bib_->column(i).set_width( gnome_config_get_int(w.c_str()) );
	}

      // connecting signals
      b_ok->clicked.connect(slot(this, &FormCitation::applySelect));
      b_cancel->clicked.connect(slot(mainAppWin, &GLyxAppWin::remove_action));

      dialog_->destroy.connect(slot(this, &FormCitation::free));

      clist_bib_->click_column.connect(slot(this, &FormCitation::sortBibList));
      clist_bib_->select_row.connect(bind(slot(this, &FormCitation::selection_toggled),
					  true, false));
      clist_bib_->unselect_row.connect(bind(slot(this, &FormCitation::selection_toggled),
					    false, false));
      // retrieving data
      vector<pair<string,string> > blist = lv_->buffer()->getBibkeyList();
      
      for ( i = 0, sz = blist.size(); i < sz; ++i )
	{
	  bibkeys.push_back(blist[i].first);
	  bibkeysInfo.push_back(blist[i].second);
	}
      
      blist.clear();      
      
      // updating list
      search();
    }
}

/*void FormCitation::show()
{
  if (!dialog_)
    {
      GtkWidget * pd = create_DiaInsertCitation();

      dialog_ = Gtk::wrap(pd);
      clist_selected_ = Gtk::wrap( GTK_CLIST( lookup_widget(pd, "clist_selected") ) );
      info_ = Gtk::wrap( GNOME_LESS( lookup_widget(pd, "info") ) );
      text_after_ = Gtk::wrap( GNOME_ENTRY( lookup_widget(pd, "text_after") ) );
      search_text_ = Gtk::wrap( GNOME_ENTRY( lookup_widget(pd, "search_text") ) );
	
      button_select_ = Gtk::wrap( GTK_BUTTON( lookup_widget(pd, "button_select") ) );
      button_unselect_ = Gtk::wrap( GTK_BUTTON( lookup_widget(pd, "button_unselect") ) );
      button_up_ = Gtk::wrap( GTK_BUTTON( lookup_widget(pd, "button_up") ) );
      button_down_ = Gtk::wrap( GTK_BUTTON( lookup_widget(pd, "button_down") ) );
      button_search_ = Gtk::wrap( GTK_BUTTON( lookup_widget(pd, "button_search") ) );

      button_regexp_ = Gtk::wrap( GTK_CHECK_BUTTON( lookup_widget(pd, "button_regexp") ) );

      paned_info_ = Gtk::wrap( GTK_PANED( lookup_widget(pd, "vpaned_info") ) );
      paned_key_ = Gtk::wrap( GTK_PANED( lookup_widget(pd, "hpaned_key") ) );
      box_keys_ =  Gtk::wrap( GTK_BOX( lookup_widget(pd, "vbox_keys") ) );

      b_ok = Gtk::wrap( GTK_BUTTON( lookup_widget(pd, "button_ok") ) );
      b_cancel = Gtk::wrap( GTK_BUTTON( lookup_widget(pd, "button_cancel") ) );

      // constructing and packing CList
      vector<string> colnames;
      colnames.push_back("INVISIBLE");
      colnames.push_back(N_("Key"));
      colnames.push_back(N_("Author(s)"));
      colnames.push_back(N_("Title"));
      colnames.push_back(N_("Year"));
      colnames.push_back(N_("Journal"));
      clist_bib_ = manage( new Gtk::CList(colnames) );
      clist_bib_->column(0).set_visiblity(false);
      
      Gtk::ScrolledWindow * sw_ = Gtk::wrap( GTK_SCROLLED_WINDOW( lookup_widget(pd, "scrolledwindow_bib") ) );
      sw_->add(*clist_bib_);

      // populating buttons with icons
      Gnome::Pixmap * p;
      p = Gtk::wrap( GNOME_PIXMAP( gnome_stock_pixmap_widget(NULL, GNOME_STOCK_PIXMAP_BACK) ) ); 
      button_select_->add(*p);
      p = Gtk::wrap( GNOME_PIXMAP( gnome_stock_pixmap_widget(NULL, GNOME_STOCK_PIXMAP_TRASH) ) ); 
      button_unselect_->add(*p);
      p = Gtk::wrap( GNOME_PIXMAP( gnome_stock_pixmap_widget(NULL, GNOME_STOCK_PIXMAP_UP) ) ); 
      button_up_->add(*p);
      p = Gtk::wrap( GNOME_PIXMAP( gnome_stock_pixmap_widget(NULL, GNOME_STOCK_PIXMAP_DOWN) ) ); 
      button_down_->add(*p);
      

      // connecting signals
      clist_bib_->click_column.connect(slot(this, &FormCitation::sortBibList));

      clist_selected_->select_row.connect(bind(slot(this, &FormCitation::selection_toggled),
					       true, true));
      clist_bib_->select_row.connect(bind(slot(this, &FormCitation::selection_toggled),
					  true, false));
      clist_selected_->unselect_row.connect(bind(slot(this, &FormCitation::selection_toggled),
						 false, true));
      clist_bib_->unselect_row.connect(bind(slot(this, &FormCitation::selection_toggled),
					    false, false));
      
      button_select_->clicked.connect(slot(this, &FormCitation::newCitation));
      button_unselect_->clicked.connect(slot(this, &FormCitation::removeCitation));
      button_up_->clicked.connect(slot(this, &FormCitation::moveCitationUp));
      button_down_->clicked.connect(slot(this, &FormCitation::moveCitationDown));

      search_text_->get_entry()->activate.connect(slot(this, &FormCitation::search));
      button_search_->clicked.connect(slot(this, &FormCitation::search));
      
      b_ok->clicked.connect(slot(this, &FormCitation::apply));
      b_ok->clicked.connect(dialog_->destroy.slot());
      b_cancel->clicked.connect(dialog_->destroy.slot());
      dialog_->destroy.connect(slot(this, &FormCitation::free));

      u_ = d_->updateBufferDependent.connect(slot(this, &FormCitation::update));
      h_ = d_->hideBufferDependent.connect(slot(this, &FormCitation::hide));

      // setting sizes of the widgets
      string path;
      string w, h;
      path  += PACKAGE "/" LOCAL_CONFIGURE_PREFIX;
      w = path + "/" + CONF_DIALOG_WIDTH + CONF_DIALOG_WIDTH_DEFAULT;
      h = path + "/" + CONF_DIALOG_HEIGTH + CONF_DIALOG_HEIGTH_DEFAULT;
      dialog_->set_usize(gnome_config_get_int(w.c_str()),
			 gnome_config_get_int(h.c_str()));

      w = path + "/" + CONF_PANE_INFO + CONF_PANE_INFO_DEFAULT;
      paned_info_->set_position( gnome_config_get_int(w.c_str()) );

      w = path + "/" + CONF_PANE_KEY + CONF_PANE_KEY_DEFAULT;
      paned_key_->set_position( gnome_config_get_int(w.c_str()) );

      int i, sz;
      for (i = 0, sz = clist_bib_->columns().size(); i < sz; ++i)
	{
	  w = path + "/" + CONF_COLUMN + "_" + tostr(i) + CONF_COLUMN_DEFAULT;
	  clist_bib_->column(i).set_width( gnome_config_get_int(w.c_str()) );
	}

      // restoring regexp setting
      w = path + "/" + CONF_REGEXP + CONF_REGEXP_DEFAULT;
      button_regexp_->set_active( (gnome_config_get_int(w.c_str()) > 0) );
      
      // ready to go...
      if (!dialog_->is_visible()) dialog_->show_all();

      update();  // make sure its up-to-date
    }
  else
    {
      Gdk_Window dialog_win(dialog_->get_window());
      dialog_win.raise();
    }
}
*/

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

void FormCitation::update()
{
  return;
  
  bibkeys.clear();
  bibkeysInfo.clear();

  clist_selected_->rows().clear();
  clist_bib_->rows().clear();

  // populating clist_bib_
  clist_bib_->freeze();

  vector<pair<string,string> > blist =
    lv_->buffer()->getBibkeyList();

  int i, sz;
  for ( i = 0, sz = blist.size(); i < sz; ++i )
    {
      bibkeys.push_back(blist[i].first);
      bibkeysInfo.push_back(blist[i].second);
    }

  blist.clear();

  for ( i = 0, sz = bibkeys.size(); i < sz; ++i )
    addItemToBibList(i);

  clist_bib_->sort();
  clist_bib_->thaw();
  // clist_bib_: done

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
  // clist_selected_: done

  text_after_->get_entry()->set_text(params.getOptions());
  
  updateButtons();
}

void FormCitation::updateButtons()
{
  bool sens;

  sens = (clist_selected_->selection().size()>0);
  button_unselect_->set_sensitive(sens);
  button_up_->set_sensitive(sens &&
			    clist_selected_->selection().operator[](0).get_row_num()>0);
  button_down_->set_sensitive(sens &&
			      clist_selected_->selection().operator[](0).get_row_num() <
			      clist_selected_->rows().size()-1);

  sens = (clist_bib_->selection().size()>0);
  button_select_->set_sensitive( (clist_bib_->selection().size()>0) );
}

void FormCitation::selection_toggled(gint            row,
				     gint            ,//column,
				     GdkEvent        * ,//event,
				     bool selected,
				     bool citeselected)
{
  if (selected)
    {
      bool keyfound = false;
      string info;
      if (citeselected)
	{
	  // lookup the record with the same key in bibkeys and show additional Info
	  int i;
	  int sz = bibkeys.size();
	  string key = clist_selected_->cell(row,0).get_text();
	  for (i=0; !keyfound && i<sz; ++i)
	    if (bibkeys[i] == key)
	      {
		info = bibkeysInfo[i];
		keyfound = true;
	      }	  
	}
      else
	{
	  // the first column in clist_bib_ contains the index
	  keyfound = true;
	  info = bibkeysInfo[ strToInt(clist_bib_->cell(row,0).get_text()) ];
	}

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

void FormCitation::removeCitation()
{
  clist_selected_->rows().remove(clist_selected_->selection().operator[](0));
  updateButtons();
}

void FormCitation::moveCitationUp()
{
  int i = clist_selected_->selection().operator[](0).get_row_num();
  clist_selected_->swap_rows( i-1, i );
  clist_selected_->row(i-1).select();
  updateButtons();
}

void FormCitation::moveCitationDown()
{
  int i = clist_selected_->selection().operator[](0).get_row_num();
  clist_selected_->swap_rows( i+1, i );
  clist_selected_->row(i+1).select();
  updateButtons();
}

void FormCitation::newCitation()
{
  // citation key is in the first column of clist_bib_ list
  vector<string> r;
  r.push_back( clist_bib_->selection().operator[](0).operator[](1).get_text() );
  clist_selected_->rows().push_back(r);
  clist_selected_->row( clist_selected_->rows().size()-1 ).select();
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
      // storing configuration
//        string path;
//        string w, h;
//        path  = PACKAGE "/" LOCAL_CONFIGURE_PREFIX;
//        w = path + "/" + CONF_DIALOG_WIDTH;
//        h = path + "/" + CONF_DIALOG_HEIGTH;

//        gnome_config_set_int(w.c_str(), dialog_->width());
//        gnome_config_set_int(h.c_str(), dialog_->height());

//        w = path + "/" + CONF_PANE_INFO;
//        gnome_config_set_int(w.c_str(), paned_key_->height());

//        w = path + "/" + CONF_PANE_KEY;
//        gnome_config_set_int(w.c_str(), box_keys_->width());

//        int i, sz;
//        for (i = 0, sz = clist_bib_->columns().size(); i < sz; ++i)
//  	{
//  	  w = path + "/" + CONF_COLUMN + "_" + tostr(i);
//  	  gnome_config_set_int(w.c_str(), clist_bib_->get_column_width(i));
//  	}

//        w = path + "/" + CONF_REGEXP;
//        gnome_config_set_int(w.c_str(), button_regexp_->get_active());

//        gnome_config_sync();

      // cleaning up
      dialog_ = NULL;
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
  int i, sz;

  contents = frontStrip( strip(params.getContents()) );
  if (!contents.empty()) contents += ", ";
  
  sz = clist_bib_->selection().size();
  for (i=0; i < sz; ++i)
    {
      if (i > 0) contents += ", ";
      contents += clist_bib_->selection().operator[](i).operator[](1).get_text();
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
				   params.getAsString().c_str() );
    }

  // save config
  text_after_->save_history();

  string path;
  string w, h;
  path  = PACKAGE "/" + LOCAL_CONFIGURE_PREFIX;

  w = path + "/" + CONF_PANE_INFO;
  gnome_config_set_int(w.c_str(), paned_info_->width() - info_->width());
  
  for (i = 0, sz = clist_bib_->columns().size(); i < sz; ++i)
    {
      w = path + "/" + CONF_COLUMN + "_" + tostr(i);
      gnome_config_set_int(w.c_str(), clist_bib_->get_column_width(i));
    }
  
  gnome_config_sync();
  
  // close dialog
  hide();
}

void FormCitation::apply()
{
  if( lv_->buffer()->isReadonly() ) return;

  string contents;
  for( unsigned int i = 0; i < clist_selected_->rows().size(); ++i )
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
				   params.getAsString().c_str() );
    }
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

  int i, sz;
  bool additem;
  for ( i = 0, sz = bibkeys.size(); i < sz; ++i )
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

  int i, sz;
  bool additem;
  for ( i = 0, sz = bibkeys.size(); i < sz; ++i )
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
