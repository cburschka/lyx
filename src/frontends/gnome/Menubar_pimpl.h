// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *       
 *          Copyright 2000 The LyX Team.
 *
 * ====================================================== */

#ifndef MENUBAR_PIMPL_H
#define MENUBAR_PIMPL_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include "gettext.h"
#include "LString.h"
#include <gnome--/app.h>
#include "frontends/Menubar.h"
#include "commandtags.h"
#include "buffer.h"

class LyXView;
class MenuBackend;
class MenuItem;
class StrPool;

#include "debug.h"

/*
  Structure containing Gtk widget and corresponding LyX action
*/
struct GtkWidgetToAction
{
public:
  GtkWidgetToAction(GtkWidget * w, int a) { widget_=w; action_=a; }
  
public:
  GtkWidget * widget_;
  int action_;
};

/** The LyX GUI independent menubar class
  The GUI interface is implemented in the corresponding Menubar_pimpl class. 
  */
class Menubar::Pimpl: public SigC::Object {
public:
  ///
  Pimpl(LyXView *, MenuBackend const &);
  ///
  ~Pimpl();
  ///
  void set(string const &);
  /// Opens a top-level submenu given its name
  void openByName(string const &);
  /// update the state of menuitems
  void update();
  /// update TOC, LOF, ... on user' request
  void updateAllLists();
  
protected:
  /// callback function
  void callback(int action);
  /// callback function used by lists
  void callbackToc(Buffer::TocItem tg);
  /// compose Gnome::UI::Array object describing the menu
  void composeUIInfo(string const & menu_name, vector<Gnome::UI::Info> & Menus, string path);
  /// compose Gnome::UI::Array object describing the TOClist
  vector<Buffer::TocItem>::const_iterator
  composeTocUIInfo(vector<Gnome::UI::Info> & menu,
		   vector<Buffer::TocItem> const & toclist,
		   vector<Buffer::TocItem>::const_iterator begin,
		   int mylevel);
  /// populate wid_act_ vector with all widgets and corresponding actions
  void connectWidgetToAction(GnomeUIInfo * guinfo);

  /// lists (toc, lof, lot, loa)
  struct ListsHolder {
    string path;
    Gnome::UI::Array lst;

    ListsHolder () { }
    ListsHolder (const ListsHolder & a) { path = a.path; lst = a.lst; }
  };
  /// populate lists
  void updateList(vector<Buffer::TocItem> *, vector<ListsHolder> *);
  
private:
  /// 
  LyXView * owner_;
  ///
  MenuBackend const * menubackend_;
  ///
  string current_menu_name_;
  Gnome::UI::Array Menu_;

  ///
  bool ignore_action_;
  int  action_;
  vector<GtkWidgetToAction> wid_act_;

  /// toc
  SigC::Connection utoc_;
  vector<ListsHolder> toc_;
};
#endif
