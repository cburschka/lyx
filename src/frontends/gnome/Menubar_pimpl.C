/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *       
 *          Copyright 2000 The LyX Team.
 *
 * ====================================================== */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include <algorithm>
#include <cctype>
#include "support/lstrings.h"
#include "support/filetools.h"
#include "support/StrPool.h"
#include "support/LAssert.h"
#include "debug.h"
#include "LyXAction.h"
#include "lyxfunc.h"
#include "func_status.h"
#include "kbmap.h"
#include "bufferlist.h"
#include "lastfiles.h"
#include "LyXView.h"
#include "MenuBackend.h"
#include "Menubar_pimpl.h"
#include "lyxtext.h"
#include "exporter.h"

#include "mainapp.h"

#include <gtk--/menu.h>

using std::endl;

// temporary solution for LyXView
extern GLyxAppWin * mainAppWin;

// Some constants
extern boost::scoped_ptr<kb_keymap> toplevel_keymap;
extern LyXAction lyxaction;
extern BufferList bufferlist;
extern LastFiles * lastfiles; 



Menubar::Pimpl::Pimpl(LyXView * view, MenuBackend const & mb) 
  : owner_(view), menubackend_(&mb), ignore_action_(false)
{
  
}

Menubar::Pimpl::~Pimpl() 
{
  if (utoc_.connected()) utoc_.disconnect();
}

void Menubar::Pimpl::set(string const & menu_name) 
{
  // if (current_menu_name_ != menu_name)  // disabled until Lastfiles and Documents are added dynamically to menu
	//{
      current_menu_name_ = menu_name;

      // clean up the lists
      toc_.clear();
      if (utoc_.connected()) utoc_.disconnect();
      
      // compose new menu
      vector<Gnome::UI::Info> menus;
      composeUIInfo(current_menu_name_, menus, "");

      // set menu
      Menu_ = menus;
      mainAppWin->set_menu(Menu_);

      // connect all menu items to correspoding action
      wid_act_.clear();
      ignore_action_ = true;
      connectWidgetToAction(Menu_.gtkobj());
      ignore_action_ = false;

      // update state of the items
      update();
      updateAllLists();
      //}
}

void Menubar::Pimpl::updateAllLists()
{
#ifdef WITH_WARNINGS
#warning Implement me! (be 20010324)
#endif
#if 0
  // update lists
  if (toc_.size() > 0)
    {
      vector<Buffer::TocItem> toclist = (owner_->view()->buffer()->getTocList())[Buffer::TOC_TOC];
      updateList(&toclist, &toc_);
    }
#endif
}

int const max_number_of_items = 25;

void Menubar::Pimpl::updateList(vector<Buffer::TocItem> * toclist, vector<ListsHolder> * pgui) 
{
  vector<ListsHolder> & gui = *pgui;
  int szGui = gui.size();
  int i;
  for (i=0; i < szGui; ++i)
    {
      int oldsz = gui[i].lst.size();
      vector<Gnome::UI::Info> menu;
      string label;

      menu.push_back(Gnome::UI::Item(Gnome::UI::Icon(GNOME_STOCK_MENU_REFRESH),
				     _("Refresh"), SigC::slot(this, &Menubar::Pimpl::updateAllLists)));

      if (toclist->size() > max_number_of_items)
	composeTocUIInfo(menu, *toclist, toclist->begin(), 0);
      else
	{
	  vector<Buffer::TocItem>::const_iterator end = toclist->end();
	  for (vector<Buffer::TocItem>::const_iterator it = toclist->begin();
	       it != end; ++it)
	    
	    {
	      label = string(4*(*it).depth,' ')+(*it).str;
	      
	      menu.push_back(Gnome::UI::Item(label,
					     SigC::bind<Buffer::TocItem>(SigC::slot(this, &Menubar::Pimpl::callbackToc), (*it)),
					     label));
	    }
	}
      
      gui[i].lst = menu;
      mainAppWin->update_menu(gui[i].path, oldsz, gui[i].lst);
    }
}

vector<Buffer::TocItem>::const_iterator
Menubar::Pimpl::composeTocUIInfo(vector<Gnome::UI::Info> & menu,
				 vector<Buffer::TocItem> const & toclist,
				 vector<Buffer::TocItem>::const_iterator begin,
				 int mylevel)
{
  string label = _("<No Name>");

  vector<Buffer::TocItem>::const_iterator end = toclist.end();
  vector<Buffer::TocItem>::const_iterator it;
  for (it = begin; it != end && (*it).depth >= mylevel; ++it)
    {
      if ( (*it).depth == mylevel &&
	   (it+1 == end || (*(it+1)).depth <= mylevel) )
	{
	  label = (*it).str;
	  menu.push_back(Gnome::UI::Item(label,
				       SigC::bind<Buffer::TocItem>(SigC::slot(this, &Menubar::Pimpl::callbackToc), (*it)),
					 label));
	}
      else
	{
	  vector<Gnome::UI::Info> submenu;
	  if ( (*it).depth == mylevel )
	    {
	      label = (*it).str;
	      submenu.push_back(Gnome::UI::Item(label,
						SigC::bind<Buffer::TocItem>(SigC::slot(this, &Menubar::Pimpl::callbackToc), (*it)),
						label));
	      ++it;    
	    }
	  it = composeTocUIInfo(submenu, toclist, it, mylevel+1);
	  menu.push_back(Gnome::UI::Menu(label,submenu,label));
	}
    }
  --it;
  return it;
}

void Menubar::Pimpl::callback(int action)
{
  // Dispatch action OR record action to local variable (see connectWidgetToAction)
  if (!ignore_action_) {
      Pimpl::update();
      owner_->getLyXFunc()->Dispatch(action);
  } else
      action_ = action;
}

void Menubar::Pimpl::callbackToc(Buffer::TocItem tg)
{
#if 0 
  if (!owner_->view()->available()) return;
  
  owner_->view()->beforeChange();
  owner_->view()->text->SetCursor( owner_->view(), tg.par, 0 );
  owner_->view()->text->sel_cursor = owner_->view()->text->cursor;
  owner_->view()->update(BufferView::SELECT|BufferView::FITCUR);
#endif

  owner_->getLyXFunc()->Dispatch(LFUN_GOTO_PARAGRAPH, tg.str);
}

void Menubar::Pimpl::composeUIInfo(string const & menu_name, vector<Gnome::UI::Info> & Menus, string rootpath)
{
  string path = rootpath;
    
  if (!menubackend_->hasMenu(menu_name))
    {
      cout << "ERROR:composeUIInfo: Unknown menu `" << menu_name
	   << "'" << endl;
      return;
    }

  Menu menu = Menu();
  menubackend_->getMenu(menu_name).expand(menu, owner_->buffer());

  for (Menu::const_iterator i = menu.begin(); i != menu.end(); ++i)
    {
      MenuItem item = (*i);
      switch(item.kind()) {

      case MenuItem::Command: {
	string label = item.label();

	path = rootpath + label;
	
	if (label.find(item.shortcut()) != string::npos)
	  label.insert(label.find(item.shortcut()), "_");

	func_status::value_type flag = owner_->getLyXFunc()->getStatus(item.action());

	Gnome::UI::Info gitem;
	SigC::Slot0<void> cback = SigC::bind<int>(SigC::slot(this, &Menubar::Pimpl::callback),item.action());

	{
	  using namespace Gnome::MenuItems;
	  int ac = item.action();
	  kb_action action;
	  string argument;
	  if (lyxaction.isPseudoAction(ac))
	    action = lyxaction.retrieveActionArg(ac, argument);
	  else
	    action = static_cast<kb_action>(ac);

	  switch(action) {
	  case LFUN_FILE_OPEN:
	    gitem = Open(cback);
	    break;
	  case LFUN_QUIT:
	    gitem = Exit(cback);
	    break;
	  case LFUN_CLOSEBUFFER:
	    gitem = Close(cback);
	    break;
	  case LFUN_MENUWRITE:
	    gitem = Save(cback);
	    break;
	  case LFUN_WRITEAS:
	    gitem = SaveAs(cback);
	    break;
	  case LFUN_BUFFER_PRINT:
	    gitem = Print(cback);
	    break;
	  case LFUN_CUT:
	    gitem = Cut(cback);
	    break;
	  case LFUN_COPY:
	    gitem = Copy(cback);
	    break;
	  case LFUN_PASTE:
	    gitem = Paste(cback);
	    break;
	  case LFUN_UNDO:
	    gitem = Gnome::MenuItems::Undo(cback); // confused with class Undo
	    break;
	  case LFUN_REDO:
	    gitem = Redo(cback);
	    break;
	  case LFUN_DIALOG_PREFERENCES:
	    gitem = Preferences(cback);
	    break;
	  case LFUN_MENUNEW:
	    gitem = Gnome::UI::Item(Gnome::UI::Icon(GNOME_STOCK_MENU_NEW),
				    label, cback, lyxaction.helpText(item.action()));
	    break;
	  case LFUN_MENUNEWTMPLT:
	    gitem = Gnome::UI::Item(Gnome::UI::Icon(GNOME_STOCK_MENU_NEW), 
				    label, cback, lyxaction.helpText(item.action()));
	    break;
	  case LFUN_MENUSEARCH:
	    gitem = Gnome::UI::Item(Gnome::UI::Icon(GNOME_STOCK_MENU_SRCHRPL), 
				    label, cback, lyxaction.helpText(item.action()));
	    break;
	  case LFUN_SPELLCHECK:
	    gitem = Gnome::UI::Item(Gnome::UI::Icon(GNOME_STOCK_MENU_SPELLCHECK), 
				    label, cback, lyxaction.helpText(item.action()));
	    break;
	  default:
	    gitem = Gnome::UI::Item(label, cback, lyxaction.helpText(item.action()));
	    break;
	  }
	}

	// first handle optional entries.
	if (item.optional() && (flag & func_status::Disabled)) {
	    lyxerr[Debug::GUI] 
		<< "Skipping optional item " << item.label() << endl; 
	    break;
	}
  	if ((flag & func_status::ToggleOn) || 
			(flag & func_status::ToggleOff))
  	  gitem = Gnome::UI::ToggleItem(label, cback, lyxaction.helpText(item.action()));

  	Menus.push_back(gitem);
	break;
      }
      
      case MenuItem::Submenu: {
	vector<Gnome::UI::Info> submenu;
	string label = item.label();

	path = rootpath + label;
	
	if (label.find(item.shortcut()) != string::npos)
	  label.insert(label.find(item.shortcut()), "_");
	composeUIInfo(item.submenu(), submenu, path + "/");
	Menus.push_back(Gnome::UI::Menu(label,submenu,label));
	break;
      }

      case MenuItem::Separator: {

	path = rootpath + "<Separator>";
	
	Menus.push_back(Gnome::UI::Separator());
	break;
      }

      case MenuItem::Toc: {
	ListsHolder t;
	t.path = path;
	toc_.push_back(t);
	break;
      }
      
      case MenuItem::Documents: 
      case MenuItem::Lastfiles: 
      case MenuItem::ViewFormats:
      case MenuItem::UpdateFormats:
      case MenuItem::ExportFormats:
			lyxerr << "Menubar::Pimpl::create_submenu: "
			  "this should not happen" << endl;
			break;
      }
    }
}

void Menubar::Pimpl::connectWidgetToAction(GnomeUIInfo * guinfo)
{
  for (; guinfo->type !=  GnomeUIInfoType(GNOME_APP_UI_ENDOFINFO); ++guinfo)
    {
      if ( ( guinfo->type == GnomeUIInfoType(GNOME_APP_UI_ITEM) ||
	     guinfo->type == GnomeUIInfoType(GNOME_APP_UI_TOGGLEITEM) ) &&
	   guinfo->moreinfo != 0 )
	{
	  (*((void(*)(void *, void *))(guinfo->moreinfo)))(0, guinfo->user_data);
	  wid_act_.push_back( GtkWidgetToAction( guinfo->widget, action_ ) );
	}
      else if ( guinfo->type == GnomeUIInfoType(GNOME_APP_UI_SUBTREE) ||
		guinfo->type == GnomeUIInfoType(GNOME_APP_UI_RADIOITEMS) )
	{
	  connectWidgetToAction(  (GnomeUIInfo *)(guinfo->moreinfo) );
	}
    }
}

void Menubar::Pimpl::update()
{
  vector<GtkWidgetToAction>::const_iterator end=wid_act_.end();
  for (vector<GtkWidgetToAction>::const_iterator i = wid_act_.begin(); i != end; ++i)
    {
      GtkWidgetToAction wa = (*i);
      func_status::value_type flag = owner_->getLyXFunc()->getStatus(wa.action_);

      if ( flag & (func_status::Disabled | func_status::Unknown) ) gtk_widget_set_sensitive(wa.widget_, false);
      else gtk_widget_set_sensitive(wa.widget_, true);

      if ( flag & func_status::ToggleOn )
	{
	  ignore_action_=true;
	  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wa.widget_), true);
	  ignore_action_=false;
	}

      if ( flag & func_status::ToggleOff )
	{
	  ignore_action_=true;
	  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wa.widget_), false);
	  ignore_action_=false;
	}
    }
}

void Menubar::Pimpl::openByName(string const &)
{
//    Pimpl::update();
}
