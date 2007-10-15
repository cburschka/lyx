/**
 * \file GuiPopupMenu.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiView.h"

#include "Action.h"
#include "GuiPopupMenu.h"
#include "GuiMenubar.h"
#include "qt_helpers.h"
#include "LyXFunc.h"
#include "MenuBackend.h"

#include "support/lstrings.h"
#include "debug.h"


using std::make_pair;
using std::string;
using std::pair;
using std::endl;


namespace lyx {
namespace frontend {

GuiPopupMenu::GuiPopupMenu(GuiMenubar * owner, MenuItem const & mi,
		bool topLevelMenu)
	: owner_(owner)
{
	name_ = mi.submenuname();

	setTitle(toqstr(getLabel(mi)));

	if (topLevelMenu)
		connect(this, SIGNAL(aboutToShow()), this, SLOT(updateView()));
}


void GuiPopupMenu::updateView()
{
	LYXERR(Debug::GUI) << "GuiPopupMenu::updateView()" << endl;
	LYXERR(Debug::GUI) << "\tTriggered menu: " << to_utf8(name_) << endl;

	clear();

	if (name_.empty())
		return;

	// Here, We make sure that theLyXFunc points to the correct LyXView.
	theLyXFunc().setLyXView(owner_->view());

	Menu const & fromLyxMenu = owner_->backend().getMenu(name_);
	owner_->backend().expand(fromLyxMenu, topLevelMenu_, owner_->view()->buffer());

	if (!owner_->backend().hasMenu(topLevelMenu_.name())) {
		LYXERR(Debug::GUI) << "\tWARNING: menu seems empty"
			<< to_utf8(topLevelMenu_.name()) << endl;
	}
	populate(this, &topLevelMenu_);
}


void GuiPopupMenu::populate(QMenu * qMenu, Menu * menu)
{
	LYXERR(Debug::GUI) << "populating menu " << to_utf8(menu->name());
	if (menu->size() == 0) {
		LYXERR(Debug::GUI) << "\tERROR: empty menu "
			<< to_utf8(menu->name()) << endl;
		return;
	}
	LYXERR(Debug::GUI) << " *****  menu entries " << menu->size() << endl;

	Menu::const_iterator m = menu->begin();
	Menu::const_iterator end = menu->end();

	for (; m != end; ++m) {

		if (m->kind() == MenuItem::Separator) {

			qMenu->addSeparator();
			LYXERR(Debug::GUI) << "adding Menubar Separator" << endl;

		} else if (m->kind() == MenuItem::Submenu) {

			LYXERR(Debug::GUI) << "** creating New Sub-Menu "
				<< to_utf8(getLabel(*m)) << endl;
			QMenu * subMenu = qMenu->addMenu(toqstr(getLabel(*m)));
			populate(subMenu, m->submenu());

		} else { // we have a MenuItem::Command

			LYXERR(Debug::GUI) << "creating Menu Item "
				<< to_utf8(m->label()) << endl;

			docstring label = getLabel(*m);
			addBinding(label, *m);

			Action * action = new Action(*(owner_->view()),
				QIcon(), toqstr(label), m->func(), QString());
			qMenu->addAction(action);
		}
	}
}


docstring const GuiPopupMenu::getLabel(MenuItem const & mi)
{
	docstring const shortcut = mi.shortcut();
	docstring label = support::subst(mi.label(),
	from_ascii("&"), from_ascii("&&"));

	if (!shortcut.empty()) {
		docstring::size_type pos = label.find(shortcut);
		if (pos != docstring::npos)
			label.insert(pos, 1, char_type('&'));
	}

	return label;
}


void GuiPopupMenu::addBinding(docstring & label, MenuItem const & mi)
{
#ifdef Q_WS_MACX
	docstring const binding = mi.binding(false);
#else
	docstring const binding = mi.binding(true);
#endif
	if (!binding.empty())
		label += '\t' + binding;
}


} // namespace frontend
} // namespace lyx

#include "GuiPopupMenu_moc.cpp"
