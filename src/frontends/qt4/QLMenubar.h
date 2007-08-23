// -*- C++ -*-
/**
 * \file qt4/QLMenubar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QLMENUBAR_H
#define QLMENUBAR_H

#include "QLPopupMenu.h"

#include <map>
#include <boost/scoped_ptr.hpp>

#include <QObject>
#include <QMenuBar>

namespace lyx {

class MenuBackend;
class Menu;
class MenuItem;

namespace frontend {

class GuiView;
class LyXView;

class QLMenubar : public QObject {
	Q_OBJECT
public:
	QLMenubar(LyXView *, MenuBackend &);

	/// opens a top-level submenu given its name
	void openByName(docstring const &);

	/// return the owning view
	GuiView * view();

	/// return the menu controller
	MenuBackend const & backend();

	/// The QMenuBar used by LyX
	QMenuBar * menuBar() const;

	/// update the state of the menuitems - not needed
	void update();

private:
	/// Initialize specific MACOS X menubar
	void macxMenuBarInit();

	/// owning view
	GuiView * owner_;

	/// menu controller
	MenuBackend & menubackend_;

	typedef std::map<docstring, QLPopupMenu *> NameMap;

	/// name to menu for openByName
	NameMap name_map_;

	/// MACOS X special menubar.
	boost::scoped_ptr<QMenuBar> mac_menubar_;
};

} // namespace frontend
} // namespace lyx

#endif // QLMENUBAR_H
