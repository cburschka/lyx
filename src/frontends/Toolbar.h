// -*- C++ -*-
/**
 * \file Toolbar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TOOLBAR_H
#define TOOLBAR_H


#include "support/std_string.h"
#include "ToolbarBackend.h"

class LyXView;

/**
 * The LyX GUI independent toolbar class
 *
 * The GUI interface is implemented in the corresponding Toolbar_pimpl class.
 */
class Toolbar {
public:
	///
	Toolbar();

	/// 
	virtual ~Toolbar();

	/// Initialize toolbar from backend
 	void init();

	/// update the state of the toolbars
	void update(bool in_math, bool in_table);

	/// update the layout combox
	virtual void setLayout(string const & layout) = 0;
	/**
	 * Populate the layout combox - returns whether we did a full
	 * update or not
	 */
	bool updateLayoutList(int textclass);
	/// Drop down the layout list
	virtual void openLayoutList() = 0;
	/// Erase the layout list
	virtual void clearLayoutList();

private:

	virtual void add(ToolbarBackend::Toolbar const & tb) = 0;

	/// update the state of the icons
	virtual void update() = 0;

	/// show or hide a toolbar
	virtual void displayToolbar(ToolbarBackend::Toolbar const & tb, 
				    bool show) = 0;

	/// Populate the layout combox.
	virtual void updateLayoutList() = 0;


	/**
	 * The last textclass layout list in the layout choice selector
	 */
	int last_textclass_;
};
#endif
