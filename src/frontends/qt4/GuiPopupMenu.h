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

#ifndef GUIPOPUPMENU_H
#define GUIPOPUPMENU_H

#include "MenuBackend.h"

#include <QMenu>

namespace lyx {
namespace frontend {

class GuiView;

/// a submenu
class GuiPopupMenu : public QMenu
{
	Q_OBJECT
public:
	///
	GuiPopupMenu(GuiView * owner, MenuItem const & mi,
		bool topLevelMenu = false);

	/// populates the menu or one of its submenu
	/// This is used as a recursive function
	void populate(QMenu * qMenu, Menu * menu);

public Q_SLOTS:
	/// populate the toplevel menu and all children
	void updateView();

private:
	/// Get a Menu item label from the menu backend
	QString label(MenuItem const & mi) const;

	/// our owning view
	GuiView * owner_;
	/// the name of this menu
	QString name_;
	/// Top Level Menu
	Menu topLevelMenu_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIPOPUPMENU_H
