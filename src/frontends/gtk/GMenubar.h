// -*- C++ -*-
/**
 * \file gtk/GMenubar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MENUBAR_PIMPL_H
#define MENUBAR_PIMPL_H

#include "frontends/Menubar.h"
#include "MenuBackend.h"
#include <vector>

class LyXView;

class GMenubar : public Menubar, public SigC::Object {
public:
	GMenubar(LyXView *, MenuBackend const &);
	~GMenubar();
	void update();
	void openByName(string const &);
private:
	void onCommandActivate(MenuItem const * item, Gtk::MenuItem * gitem);
	void onSubMenuActivate(MenuItem const * item, Gtk::MenuItem * gitem);
	bool submenuDisabled(MenuItem const * item);
	Gtk::MenuBar menubar_;
	LyXView * view_;
	std::vector<Glib::ustring> mainMenuNames_;
};

#endif
