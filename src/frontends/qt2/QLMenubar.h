// -*- C++ -*-
/**
 * \file qt2/QLMenubar.h
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

#include "frontends/Menubar.h"

#include <map>

class LyXView;
class MenuBackend;
class QMenuBar;

namespace lyx {
namespace frontend {

class QLPopupMenu;
class QtView;

class QLMenubar : public Menubar {
public:
	QLMenubar(LyXView *, MenuBackend &);

	/// opens a top-level submenu given its name
	void openByName(std::string const &);

	/// update the state of the menuitems - not needed
	void update();

	/// return the owning view
	QtView * view();

	/// return the menu controller
	MenuBackend const & backend();
private:
	/// owning view
	QtView * owner_;

	/// menu controller
	MenuBackend & menubackend_;

	typedef std::map<std::string, QLPopupMenu *> NameMap;

	/// name to menu for openByName
	NameMap name_map_;

	/// The QMenuBar used by LyX
	QMenuBar * menuBar() const;

#ifdef Q_WS_MACX
	boost::scoped_ptr<QMenuBar> menubar_;
#endif
};

} // namespace frontend
} // namespace lyx

#endif // QLMENUBAR_H
