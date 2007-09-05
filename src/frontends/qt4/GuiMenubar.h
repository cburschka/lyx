// -*- C++ -*-
/**
 * \file GuiMenubar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIMENUBAR_H
#define GUIMENUBAR_H

#include "GuiPopupMenu.h"

#include <map>
#include <boost/scoped_ptr.hpp>

#include <QObject>
#include <QMenuBar>

namespace lyx {

class MenuBackend;
class Menu;
class MenuItem;

namespace frontend {

class GuiViewBase;
class LyXView;

class GuiMenubar : public QObject
{
	Q_OBJECT
public:
	GuiMenubar(LyXView *, MenuBackend &);

	/// opens a top-level submenu given its name
	void openByName(docstring const &);

	/// return the owning view
	GuiViewBase * view() { return owner_; }

	/// return the menu controller
	MenuBackend const & backend() { return menubackend_; }

	/// The QMenuBar used by LyX
	QMenuBar * menuBar() const;

	/// update the state of the menuitems - not needed
	void updateView();

private:
	/// Initialize specific MACOS X menubar
	void macxMenuBarInit();

	/// owning view
	GuiViewBase * owner_;

	/// menu controller
	MenuBackend & menubackend_;

	typedef std::map<docstring, GuiPopupMenu *> NameMap;

	/// name to menu for openByName
	NameMap name_map_;

	/// MACOS X special menubar.
	boost::scoped_ptr<QMenuBar> mac_menubar_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIMENUBAR_H
