// -*- C++ -*-
/**
 * \file Toolbar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef TOOLBAR_H
#define TOOLBAR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

class LyXView;
class ToolbarDefaults;


/** The LyX GUI independent toolbar class
  The GUI interface is implemented in the corresponding Toolbar_pimpl class.
  */
class Toolbar {
public:
	///
	Toolbar(LyXView * o, int x, int y, ToolbarDefaults const &);

	///
	~Toolbar();

	/// update the state of the icons
	void update();

	/// update the layout combox
	void setLayout(string const & layout);
	/**
	 * Populate the layout combox - returns whether we did a full
	 * update or not
	 */
	bool updateLayoutList(int textclass);
	/// Drop down the layout list
	void openLayoutList();
	/// Erase the layout list
	void clearLayoutList();

	/// Compaq cxx 6.5 requires this to be public
	struct Pimpl;
private:
	///
	friend struct Toolbar::Pimpl;
	///
	Pimpl * pimpl_;

	/**
	 * The last textclass layout list in the layout choice selector
	 */
	int last_textclass_;
};
#endif
