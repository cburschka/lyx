/**
 * \file GMenubar.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <gtkmm.h>

#include "GMenubar.h"
#include "GView.h"
#include "debug.h"
#include "lyxfunc.h"

using std::string;

namespace lyx {
namespace frontend {

namespace
{

class LyxMenu : public Gtk::Menu {
public:
	LyxMenu() { menu_.reset(new ::Menu); }

	::Menu& getBackMenu() { return *menu_.get(); }

	void clearBackMenu() { menu_.reset(new ::Menu); }
private:
	std::auto_ptr< ::Menu > menu_;
};


Glib::ustring labelTrans(string const & label, string const & shortcut)
{
	string labelN = label;
	string::size_type i = label.find(shortcut);
	if (i == string::npos)
		return Glib::locale_to_utf8(label);
	labelN.insert(i, "_");
	return Glib::locale_to_utf8(labelN);
}


void ClearMenu(Gtk::MenuShell * menu)
{
	Gtk::Menu_Helpers::MenuList::iterator m = menu->items().begin();
	Gtk::Menu_Helpers::MenuList::iterator end = menu->items().end();
	Gtk::Menu * subMenu;
	for (; m != end; ++m) {
		if ((subMenu = m->get_submenu()) != 0) {
			ClearMenu(subMenu);
			delete subMenu;
		}
	}
	menu->items().clear();
}


}


GMenubar::GMenubar(LyXView * lyxView, MenuBackend const & /*menuBackend*/) :
	view_(lyxView)
{
	GView * gview = static_cast<GView*>(lyxView);
	Menu const & menu = menubackend.getMenubar();
	Menu::const_iterator i = menu.begin();
	Menu::const_iterator end = menu.end();
	for (; i != end; ++i) {
		if (i->kind() != MenuItem::Submenu) {
			lyxerr << "ERROR: GMenubar::createMenubar:"
				" only submenus can appear in a menubar"
			       << std::endl;
			continue;
		}
		Gtk::Menu * gmenu = new LyxMenu;
		menubar_.items().push_back(
			Gtk::Menu_Helpers::MenuElem(
				labelTrans(i->label(), i->shortcut()),
				*gmenu));
		menubar_.items().back().signal_activate().connect(
			sigc::bind(sigc::mem_fun(*this, &GMenubar::onSubMenuActivate), &(*i),
				   &menubar_.items().back()));
		mainMenuNames_.push_back(i->submenuname());
	}
	menubar_.show();
	gview->getBox(GView::Top).children().push_back(
		Gtk::Box_Helpers::Element(menubar_, Gtk::PACK_SHRINK));
}


GMenubar::~GMenubar()
{
	ClearMenu(&menubar_);
}


void GMenubar::update()
{
}


void GMenubar::openByName(string const & name)
{
	Glib::ustring uname = Glib::locale_to_utf8(name);
	std::vector<Glib::ustring>::iterator it =
		std::find(mainMenuNames_.begin(), mainMenuNames_.end(),
			  uname);
	if (it != mainMenuNames_.end()) {
		Gtk::MenuItem& mitem = menubar_.items()[it - mainMenuNames_.begin()];
		mitem.select();
		mitem.activate();
		return;
	}
	lyxerr << "GMenubar::openByName: menu "
	       << name << " not found" << std::endl;
}


bool GMenubar::submenuDisabled(MenuItem const * item)
{
	Menu & from = menubackend.getMenu(item->submenuname());
	Menu to;
	menubackend.expand(from, to, view_);
	Menu::const_iterator i = to.begin();
	Menu::const_iterator end = to.end();
	for (; i != end; ++i) {
		switch (i->kind()) {
		case MenuItem::Submenu:
			if (!submenuDisabled(&(*i)))
				return false;
			break;
		case MenuItem::Command:
		{
			FuncStatus const flag =
				view_->getLyXFunc().getStatus(i->func());
			if (flag.enabled())
				return false;
			break;
		}
		default:
			break;
		}
	}
	return true;
}


void GMenubar::onSubMenuActivate(MenuItem const * item,
				 Gtk::MenuItem * gitem)
{
	Gtk::Menu * gmenu = gitem->get_submenu();
	ClearMenu(gmenu);
	LyxMenu * lyxmenu = static_cast<LyxMenu*>(gmenu);
	lyxmenu->clearBackMenu();
	Menu * fmenu = &menubackend.getMenu(item->submenuname());
	menubackend.expand(*fmenu, lyxmenu->getBackMenu(), view_);
	Menu::const_iterator i = lyxmenu->getBackMenu().begin();
	Menu::const_iterator end = lyxmenu->getBackMenu().end();
	Gtk::Menu * gmenu_new;
	for (; i != end; ++i) {
		switch (i->kind()) {
		case MenuItem::Submenu:
			gmenu_new = new LyxMenu;
			gmenu->items().push_back(
				Gtk::Menu_Helpers::MenuElem(
					labelTrans(i->label(), i->shortcut()),
					*gmenu_new));
			gmenu->items().back().signal_activate().connect(
				sigc::bind(sigc::mem_fun(*this, &GMenubar::onSubMenuActivate),
					   &(*i),
					   &gmenu->items().back()));
			if (submenuDisabled(&(*i)))
				gmenu->items().back().set_sensitive(false);
			break;
		case MenuItem::Command:
		{
#ifdef WITH_WARNINGS
#warning Bindings are not inserted into the menu labels here. (Lgb)
#endif
			FuncStatus const flag =
				view_->getLyXFunc().getStatus(i->func());
			bool on = flag.onoff(true);
			bool off = flag.onoff(false);

			if (on || off) {
				gmenu->items().push_back(
					Gtk::Menu_Helpers::CheckMenuElem(
						labelTrans(i->label(),
							   i->shortcut())));
				Gtk::CheckMenuItem& citem =
					static_cast<Gtk::CheckMenuItem&>(
						gmenu->items().back());
				citem.set_active(on);
			} else {
				gmenu->items().push_back(
					Gtk::Menu_Helpers::MenuElem(
						labelTrans(i->label(),
							   i->shortcut())));
			}
			Gtk::MenuItem & item = gmenu->items().back();
			item.signal_activate().connect(
				sigc::bind(sigc::mem_fun(*this, &GMenubar::onCommandActivate),
					   &(*i), &item));
			if (!flag.enabled())
				item.set_sensitive(false);
			break;
		}
		case MenuItem::Separator:
			gmenu->items().push_back(
				Gtk::Menu_Helpers::SeparatorElem());
			break;
		default:
			lyxerr << "GMenubar::create_submenu: "
				"this should not happen" << std::endl;
			break;
		}
	}
}


void GMenubar::onCommandActivate(MenuItem const * item,
				       Gtk::MenuItem * /*gitem*/)
{
	view_->getLyXFunc().dispatch(item->func(), true);
}

} // namespace frontend
} // namespace lyx
