/**
 * \file GMenubar.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GMenubar.h"
#include "GView.h"
#include "ghelpers.h"

#include "ToolbarBackend.h" // for getIcon

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


void GMenubar::onSubMenuActivate(MenuItem const * item,
				 Gtk::MenuItem * gitem)
{
	Gtk::Menu * gmenu = gitem->get_submenu();
	ClearMenu(gmenu);
	LyxMenu * lyxmenu = static_cast<LyxMenu*>(gmenu);
	lyxmenu->clearBackMenu();
	Menu * fmenu = item->submenuname().empty() ?
		item->submenu() :
		&menubackend.getMenu(item->submenuname());

	// Choose size for icons on command items
	int iconwidth = 16;
	int iconheight = 16;
	Gtk::IconSize::lookup(Gtk::ICON_SIZE_MENU, iconwidth, iconheight);

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
			if (!i->status().enabled())
				gmenu->items().back().set_sensitive(false);
			break;
		case MenuItem::Command:
		{
			FuncStatus const flag = i->status();
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
				// Choose an icon from the funcrequest
				Gtk::BuiltinStockID stockID = getGTKStockIcon(i->func());
				Gtk::Image * image = NULL;
				// Prefer stock graphics
				if (stockID != Gtk::Stock::MISSING_IMAGE) {
					image = Gtk::manage(new Gtk::Image(stockID, Gtk::ICON_SIZE_MENU));
				} else {
					Glib::ustring xpmName =
						Glib::locale_to_utf8(toolbarbackend.getIcon(i->func()));
					if (xpmName.find("unknown.xpm") == -1) {
						// Load icon and shrink it for menu size
						Glib::RefPtr<Gdk::Pixbuf> bigicon =
							Gdk::Pixbuf::create_from_file(xpmName);
						Glib::RefPtr<Gdk::Pixbuf> smallicon =
							bigicon->scale_simple(iconwidth,iconheight,Gdk::INTERP_TILES);
						image = Gtk::manage(new Gtk::Image(smallicon));
					}
				}

				Gtk::ImageMenuItem * item = Gtk::manage(new Gtk::ImageMenuItem);
				if (image)
					item->set_image(*image);

				// This hbox is necessary because add_accel_label is protected,
				// and even if you subclass Gtk::MenuItem then add_accel_label
				// doesn't do what you'd expect.
				Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox);
				Gtk::Label * label1 = Gtk::manage(new Gtk::Label(
					labelTrans(i->label(), i->shortcut()), true));
				Gtk::Label * label2 = Gtk::manage(new Gtk::Label(
					"   " + i->binding(), false));
				hbox->pack_start(*label1, false, false, 0);
				hbox->pack_end(*label2, false, false, 0);
				item->add(*hbox);

				gmenu->append(*item);
				item->show_all();
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
	view_->getLyXFunc().dispatch(item->func());
}

} // namespace frontend
} // namespace lyx
