// -*- C++ -*-
/**
 * \file qt2/Toolbar_pimpl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef TOOLBAR_PIMPL_H
#define TOOLBAR_PIMPL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "frontends/Toolbar.h"

#include <qobject.h>
#include <qtoolbutton.h>

#include <boost/scoped_ptr.hpp>
#include <map>
#include <vector>

class QtView;
class QToolBar;
class QLComboBox;
class ToolbarProxy;

struct Toolbar::Pimpl {

	friend class ToolbarProxy;

public:
	Pimpl(LyXView * o, int x, int y);

	~Pimpl();

	/// add a new button to the toolbar.
    	void add(int action);

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
