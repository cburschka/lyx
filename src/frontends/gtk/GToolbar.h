// -*- C++ -*-
/**
 * \file GToolbar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TOOLBAR_PIMPL_H
#define TOOLBAR_PIMPL_H

#include <gtkmm.h>
#include "frontends/Toolbar.h"
#include "ToolbarBackend.h"


class GToolbar : public Toolbar, public SigC::Object
{
public:
	GToolbar(LyXView * o, int x, int y);

	~GToolbar();

	// add a new toolbar
	void add(ToolbarBackend::Toolbar const & tb);

	/// add a new button to the toolbar.
	void add(Gtk::Toolbar * toolbar,
		 ToolbarBackend::Item const & item);

	/// display toolbar, not implemented
	void displayToolbar(ToolbarBackend::Toolbar const & tb, bool show);

	/// update the state of the icons
	void update();

	/// select the right layout in the combox
	void setLayout(std::string const & layout);

	/// Populate the layout combox; re-do everything if force is true.
	void updateLayoutList();

	/// Drop down the layout list
	void openLayoutList();

	/// Erase the layout list
	void clearLayoutList();
private:
	void onButtonClicked(FuncRequest);
	void onLayoutSelected();
	Gtk::VBox vbox_;
	std::vector<Gtk::Toolbar*> toolbars_;
	Gtk::Combo combo_;
	LyXView * view_;
	bool internal_;
};

#endif
