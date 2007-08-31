// -*- C++ -*-
/**
 * \file GuiPopupMenu.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QLPOPUPMENU_H
#define QLPOPUPMENU_H

#include <QMenu>

#include "FuncRequest.h"
#include "MenuBackend.h"

namespace lyx {
namespace frontend {

class GuiMenubar;

/// a submenu
class GuiPopupMenu : public QMenu {
	Q_OBJECT
public:

	GuiPopupMenu(GuiMenubar * owner, MenuItem const & mi, bool topLevelMenu=false);

	/// populates the menu or one of its submenu
	/// This is used as a recursive function
	void populate(QMenu* qMenu, Menu * menu);

public Q_SLOTS:
	/// populate the toplevel menu and all children
	void update();

private:

	/// our owning menubar
	GuiMenubar * owner_;

	/// the name of this menu
	docstring name_;

private:
	/// Get a Menu item label from the menu backend
	docstring const getLabel(MenuItem const & mi);

	/// add binding keys a the menu item label.
	void addBinding(docstring & label, MenuItem const & mi);

	/// Top Level Menu
	Menu topLevelMenu_;
};

} // namespace frontend
} // namespace lyx

#endif // QLPOPUPMENU_H
