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
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GMenubar.h"
#include "GView.h"
#include "ghelpers.h"

#include "ToolbarBackend.h" // for getIcon

#include "debug.h"
#include "support/lstrings.h"
#include "support/docstring.h"
#include "lyxfunc.h"

using lyx::char_type;
using lyx::docstring;

using std::string;

namespace lyx {

using support::subst;

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


Glib::ustring labelTrans(docstring const & label_src,
			 docstring const & shortcut)
{
	docstring label = subst(label_src,
				lyx::from_ascii("_"),
				lyx::from_ascii("__"));
	docstring::size_type i = label.find(shortcut);
	if (i == docstring::npos)
		return lyx::to_utf8(label);
	label.insert(i, lyx::from_ascii("_"));
	return lyx::to_utf8(label);
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
		mainMenuNames_.push_back(lyx::to_utf8(i->submenuname()));
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


void GMenubar::openByName(docstring const & name)
{
	Glib::ustring uname = lyx::to_utf8(name);
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
	       << lyx::to_utf8(name) << " not found" << std::endl;
}


void GMenubar::onSubMenuActivate(MenuItem const * item,
				 Gtk::MenuItem * gitem)
{
	Gtk::Menu * gmenu = gitem->get_submenu();
	ClearMenu(gmenu);
	LyxMenu * lyxmenu = static_cast<LyxMenu*>(gmenu);
	lyxmenu->clearBackMenu();

	Menu * fmenu;
	Menu::const_iterator i;
	Menu::const_iterator end;
	if(!item->submenuname().empty()) {
		fmenu = &menubackend.getMenu(item->submenuname());
		menubackend.expand(*fmenu, lyxmenu->getBackMenu(), view_->buffer());
		i = lyxmenu->getBackMenu().begin();
		end = lyxmenu->getBackMenu().end();
	} else {
		fmenu = item->submenu();
		i = fmenu->begin();
		end = fmenu->end();
	}

	// Choose size for icons on command items
	int iconwidth = 16;
	int iconheight = 16;
	Gtk::IconSize::lookup(Gtk::ICON_SIZE_MENU, iconwidth, iconheight);

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
				Gtk::CheckMenuItem& checkitem =
					static_cast<Gtk::CheckMenuItem&>(
						gmenu->items().back());
				checkitem.set_active(on);
			} else {
				// Choose an icon from the funcrequest
				Gtk::Image * image = getGTKIcon(i->func(), Gtk::ICON_SIZE_MENU);
				if (!image) {
					// ENCODING, FIXME: does Pixbuf::create_from_file really
					// want UTF-8, or does it want filename encoding?  Is
					// the backend string really in locale encoding?
					// This shouldn't break as long as filenames are ASCII
					Glib::ustring xpmName =
						Glib::locale_to_utf8(toolbarbackend.getIcon(i->func()));
					if (xpmName.find("unknown.xpm") == Glib::ustring::npos) {
						// Load icon and shrink it for menu size
						Glib::RefPtr<Gdk::Pixbuf> bigicon =
							Gdk::Pixbuf::create_from_file(xpmName);
						Glib::RefPtr<Gdk::Pixbuf> smallicon =
							bigicon->scale_simple(iconwidth,iconheight,Gdk::INTERP_TILES);
						image = Gtk::manage(new Gtk::Image(smallicon));
					}
				}

				Gtk::ImageMenuItem * imgitem = Gtk::manage(new Gtk::ImageMenuItem);
				if (image)
					imgitem->set_image(*image);

				// This hbox is necessary because add_accel_label is protected,
				// and even if you subclass Gtk::MenuItem then add_accel_label
				// doesn't do what you'd expect.
				Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox);
				Gtk::Label * label1 = Gtk::manage(new Gtk::Label(
					labelTrans(i->label(), i->shortcut()), true));
				Gtk::Label * label2 =
					Gtk::manage(new Gtk::Label(
								   "   " + lyx::to_utf8(i->binding()), false));
				hbox->pack_start(*label1, false, false, 0);
				hbox->pack_end(*label2, false, false, 0);
				imgitem->add(*hbox);

				gmenu->append(*imgitem);
				imgitem->show_all();
			}
			Gtk::MenuItem & newitem = gmenu->items().back();
			newitem.signal_activate().connect(
				sigc::bind(sigc::mem_fun(*this, &GMenubar::onCommandActivate),
					   &(*i), &newitem));
			if (!flag.enabled())
				newitem.set_sensitive(false);
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
	view_->dispatch(item->func());
}

} // namespace frontend
} // namespace lyx
