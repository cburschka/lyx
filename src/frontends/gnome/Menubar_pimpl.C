// -*- C++ -*-
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
#include "kbmap.h"
#include "bufferlist.h"
#include "lastfiles.h"
#include "LyXView.h"
#include "MenuBackend.h"
#include "Menubar_pimpl.h"

#include "mainapp.h"

using SigC::slot;
using SigC::bind;

using std::endl;

// temporary solution for LyXView
extern GLyxAppWin * mainAppWin;

// Some constants
extern kb_keymap * toplevel_keymap;
extern LyXAction lyxaction;
extern BufferList bufferlist;
extern LastFiles * lastfiles; 



Menubar::Pimpl::Pimpl(LyXView * view, MenuBackend const & mb) 
  : owner_(view), menubackend_(&mb), ignore_action_(false)
{
	// Should we do something here?
}

Menubar::Pimpl::~Pimpl() 
{
	// Should we do something here?
}

void Menubar::Pimpl::set(string const & menu_name) 
{
  // if (current_menu_name_ != menu_name)  // disabled until Lastfiles and Documents are added dynamically to menu
    {
      current_menu_name_ = menu_name;
      // compose new menu
      vector<Gnome::UI::Info> menus;
      composeUIInfo(current_menu_name_, menus);

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
    }
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

void Menubar::Pimpl::composeUIInfo(string const & menu_name, vector<Gnome::UI::Info> & Menus)
{
  if (!menubackend_->hasMenu(menu_name))
    {
      cout << "ERROR:composeUIInfo: Unknown menu `" << menu_name
	   << "'" << endl;
      return;
    }

  Menu menu = menubackend_->getMenu(menu_name);

  for (Menu::const_iterator i = menu.begin(); i != menu.end(); ++i)
    {
      MenuItem item = (*i);
      switch(item.kind()) {

      case MenuItem::Command: {
	string label = item.label();
	if (label.find(item.shortcut()) != string::npos)
	  label.insert(label.find(item.shortcut()), "_");

	LyXFunc::func_status flag = owner_->getLyXFunc()->getStatus(item.action());

	Gnome::UI::Info gitem;
	SigC::Slot0<void> cback = bind<int>(slot(this, &Menubar::Pimpl::callback),item.action());

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
	  case LFUN_MENUOPEN:
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
	  case LFUN_MENUWRITEAS:
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
	if (item.optional() && (flag & LyXFunc::Disabled)) {
	    lyxerr[Debug::GUI] 
		<< "Skipping optional item " << item.label() << endl; 
	    break;
	}
  	if ((flag & LyXFunc::ToggleOn) || (flag & LyXFunc::ToggleOff))
  	  gitem = Gnome::UI::ToggleItem(label, cback, lyxaction.helpText(item.action()));

  	Menus.push_back(gitem);
	break;
      }
      
      case MenuItem::Submenu: {
	vector<Gnome::UI::Info> submenu;
	string label = item.label();
	if (label.find(item.shortcut()) != string::npos)
	  label.insert(label.find(item.shortcut()), "_");
	composeUIInfo(item.submenu(), submenu);
	Menus.push_back(Gnome::UI::Menu(label,submenu,label));
	break;
      }

      case MenuItem::Separator: {
	Menus.push_back(Gnome::UI::Separator());
	break;
      }

      case MenuItem::Lastfiles: {
	int ii = 1;
	for (LastFiles::const_iterator cit = lastfiles->begin();
	     cit != lastfiles->end() && ii < 10; ++cit, ++ii)
	  {
	    int action = lyxaction.getPseudoAction(LFUN_FILE_OPEN, (*cit));
	    string label = "_" + tostr(ii) + ". " + MakeDisplayPath((*cit),30);

	    Menus.push_back(Gnome::UI::Item(label,
					    bind<int>(slot(this, &Menubar::Pimpl::callback), action),
					    label));
	  }
	break;
      }
      
      case MenuItem::Documents: {
	std::vector<string> names = bufferlist.getFileNames();

	for (std::vector<string>::const_iterator cit = names.begin();
	     cit != names.end() ; ++cit)
	  {
	    int action = lyxaction.getPseudoAction(LFUN_SWITCHBUFFER, *cit);
	    string label = MakeDisplayPath(*cit, 30);

	    Menus.push_back(Gnome::UI::Item(label,
					    bind<int>(slot(this, &Menubar::Pimpl::callback), action),
					    label));
	    
	  }
	break;
      }
      }
    }
}

void Menubar::Pimpl::connectWidgetToAction(GnomeUIInfo * guinfo)
{
  for (; guinfo->type !=  GnomeUIInfoType(GNOME_APP_UI_ENDOFINFO); ++guinfo)
    {
      if ( guinfo->type == GnomeUIInfoType(GNOME_APP_UI_ITEM) ||
	   guinfo->type == GnomeUIInfoType(GNOME_APP_UI_TOGGLEITEM) )
	{
	  (*((void(*)(void *, void *))(guinfo->moreinfo)))(NULL, guinfo->user_data);
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
      LyXFunc::func_status flag = owner_->getLyXFunc()->getStatus(wa.action_);

      if ( flag & (LyXFunc::Disabled | LyXFunc::Unknown) ) gtk_widget_set_sensitive(wa.widget_, false);
      else gtk_widget_set_sensitive(wa.widget_, true);

      if ( flag & LyXFunc::ToggleOn )
	{
	  ignore_action_=true;
	  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wa.widget_), true);
	  ignore_action_=false;
	}

      if ( flag & LyXFunc::ToggleOff )
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
