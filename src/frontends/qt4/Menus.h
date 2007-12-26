// -*- C++ -*-
/**
 * \file Menus.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MENUS_H
#define MENUS_H

#include "MenuBackend.h"

#include <QObject>
#include <QHash>

namespace lyx {
namespace frontend {

class GuiView;
class GuiPopupMenu;
class GuiView;

class Menus : public QObject, public MenuBackend
{
	Q_OBJECT
public:
	Menus() {}

	///
	void fillMenuBar(GuiView * view);

	/// opens a top-level submenu given its name
	void openByName(QString const & name);

	/// update the state of the menuitems - not needed
	void updateView();

private:
	/// Initialize specific MACOS X menubar
	void macxMenuBarInit(GuiView * view);

	typedef QHash<QString, GuiPopupMenu *> NameMap;

	/// name to menu for openByName
	NameMap name_map_;
};

} // namespace frontend
} // namespace lyx

#endif // MENUS_H
