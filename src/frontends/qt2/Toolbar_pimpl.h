// -*- C++ -*-
/**
 * \file Toolbar_pimpl.h
 * Copyright 2002 the LyX Team
 * Copyright 1996-2001 Lars Gullik Bjønnes
 * Read the file COPYING
 *
 * \author Lars Gullik Bjønnes, larsbj@lyx.org
 */

#ifndef TOOLBAR_PIMPL_H
#define TOOLBAR_PIMPL_H

#include <config.h>
#include <map>
#include <vector>

#include <boost/smart_ptr.hpp>
 
#include "frontends/Toolbar.h"

#ifdef __GNUG__
#pragma interface
#endif

#include <qobject.h>
#include <qtoolbutton.h>
 
class QtView;
class QToolBar;
class QLComboBox;
class ToolbarProxy;

struct Toolbar::Pimpl {
 
	friend class ToolbarProxy;
 
public:
	///
	Pimpl(LyXView * o, Dialogs &, int x, int y);
	///
	~Pimpl();
	
	/// (re)sets the toolbar
	void set(bool doingmain = false);

	void reset() { }
 
	/** this is to be the entry point to the toolbar
	    frame, where you can change the toolbar realtime.
	*/
	void edit();
	/// add a new button to the toolbar.
    	void add(int, bool doclean = true);
	/// update the state of the icons
	void update();

	/// select the right layout in the combox
	void setLayout(string const & layout);
	/// Populate the layout combox; re-do everything if force is true.
	void updateLayoutList(bool force);
	/// Drop down the layout list
	void openLayoutList();
	/// Erase the layout list
	void clearLayoutList();

private:
	void changed_layout(string const & sel);

	void button_selected(QToolButton * button);
 
	QtView * owner_; 

	boost::scoped_ptr<ToolbarProxy> proxy_;

	std::vector<QToolBar *> toolbars_;

	QLComboBox * combo_;

	typedef std::map<QToolButton *, int> ButtonMap;

	ButtonMap map_;
 
};

 
// moc is mind-numbingly stupid
class ToolbarProxy : public QObject {
	Q_OBJECT

public:
	ToolbarProxy(Toolbar::Pimpl & owner)
		: owner_(owner) { };
 
public slots:
 
	void layout_selected(const QString & str) {
		owner_.changed_layout(str.latin1());
	}

	void button_selected() {
		owner_.button_selected(
			const_cast<QToolButton *>(
			static_cast<QToolButton const *>(sender()))
		);
	}

private:
	Toolbar::Pimpl & owner_;
};
 
#endif
