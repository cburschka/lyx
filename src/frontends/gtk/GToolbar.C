/**
 * \file GToolbar.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GToolbar.h"
#include "GView.h"

#include "buffer.h"
#include "bufferparams.h"
#include "debug.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "lyxfunc.h"

using std::string;

namespace lyx {
namespace frontend {

namespace {

GView::Position getPosition(ToolbarBackend::Flags const & flags)
{
	if (flags & ToolbarBackend::TOP)
		return GView::Top;
	if (flags & ToolbarBackend::BOTTOM)
		return GView::Bottom;
	if (flags & ToolbarBackend::LEFT)
		return GView::Left;
	if (flags & ToolbarBackend::RIGHT)
		return GView::Right;
	return GView::Top;
}


LyXTextClass const & getTextClass(LyXView const & lv)
{
	return lv.buffer()->params().getLyXTextClass();
}


void comboClear(Gtk::Combo & combo)
{
	std::vector<Glib::ustring> strings;
	strings.push_back("");
	combo.set_popdown_strings(strings);
}


bool comboIsEmpty(Gtk::Combo & combo)
{
	std::vector<Glib::ustring> strings = combo.get_popdown_strings();
	return (strings.empty() || (strings.size() == 1 && strings[0] == ""));
}

char const * gToolData = "tool_data";

} // namespace anon


GLayoutBox::GLayoutBox(LyXView & owner,
		       Gtk::Toolbar & toolbar,
		       FuncRequest const & func)
	: owner_(owner),
	  internal_(false)
{
	combo_.set_value_in_list();
	combo_.get_entry()->set_editable(false);
	combo_.unset_flags(Gtk::CAN_FOCUS | Gtk::CAN_DEFAULT);
	combo_.get_entry()->unset_flags(Gtk::CAN_FOCUS | Gtk::CAN_DEFAULT);
	comboClear(combo_);

	combo_.get_entry()->signal_changed().connect(
		sigc::mem_fun(*this,&GLayoutBox::selected));

	combo_.show();

	combo_.set_data(
		gToolData,
		reinterpret_cast<void*>(&const_cast<FuncRequest &>(func)));

	Gtk::ToolItem * toolitem = Gtk::manage(new Gtk::ToolItem);
	toolitem->add(combo_);
	toolbar.insert(*toolitem,-1);
}

void GLayoutBox::set(string const & layout)
{
	LyXTextClass const & tc = getTextClass(owner_);

	internal_ = true;
	combo_.get_entry()->set_text(tc[layout]->name());
	internal_ = false;
}


void GLayoutBox::update()
{
	LyXTextClass const & tc = getTextClass(owner_);

	std::vector<Glib::ustring> strings;

	LyXTextClass::const_iterator it = tc.begin();
	LyXTextClass::const_iterator const end = tc.end();
	for (; it != end; ++it)
		if ((*it)->obsoleted_by().empty())
			strings.push_back(
				Glib::locale_to_utf8((*it)->name()));
	internal_ = true;
	combo_.set_popdown_strings(strings);
	internal_ = false;
}


void GLayoutBox::clear()
{
	internal_ = true;
	comboClear(combo_);
	internal_ = false;
}


void GLayoutBox::open()
{
	combo_.get_list()->activate();
}


void GLayoutBox::setEnabled(bool sensitive)
{
	combo_.set_sensitive(sensitive);
}


void GLayoutBox::selected()
{
	if (internal_)
		return;

	string layoutGuiName = combo_.get_entry()->get_text();
	// we get two signal, one of it is empty and useless
	if (layoutGuiName.empty())
		return;
	LyXTextClass const & tc = getTextClass(owner_);

	LyXTextClass::const_iterator it = tc.begin();
	LyXTextClass::const_iterator const end = tc.end();
	for (; it != end; ++it) {
		string const & name = (*it)->name();
		if (name == layoutGuiName) {
			owner_.getLyXFunc().dispatch(
				FuncRequest(LFUN_LAYOUT, name),
				true);
			return;
		}
	}
	lyxerr << "ERROR (GLayoutBox::selected): layout not found! name: "
	       << layoutGuiName << std::endl;
}

} // namespace frontend
} // namespace lyx


Toolbars::ToolbarPtr make_toolbar(ToolbarBackend::Toolbar const & tbb,
				  LyXView & owner)
{
	using lyx::frontend::GToolbar;
	return Toolbars::ToolbarPtr(new GToolbar(tbb, owner));
}

namespace lyx {
namespace frontend {

GToolbar::GToolbar(ToolbarBackend::Toolbar const & tbb, LyXView & owner)
	: owner_(dynamic_cast<GView &>(owner))
{
	ToolbarBackend::item_iterator it = tbb.items.begin();
	ToolbarBackend::item_iterator end = tbb.items.end();
	for (; it != end; ++it)
		add(it->first, it->second);

	toolbar_.set_toolbar_style(Gtk::TOOLBAR_ICONS);
	toolbar_.show_all();

	GView::Position const position = getPosition(tbb.flags);

	if (position == GView::Left || position == GView::Right)
		toolbar_.set_orientation(Gtk::ORIENTATION_VERTICAL);

	owner_.getBox(position).children().push_back(
		Gtk::Box_Helpers::Element(toolbar_, Gtk::PACK_SHRINK));

	tooltips_.enable();
}

void GToolbar::add(FuncRequest const & func, string const & tooltip)
{
	switch (func.action) {
	case ToolbarBackend::SEPARATOR: {
		Gtk::SeparatorToolItem * space =
			Gtk::manage(new Gtk::SeparatorToolItem);
		toolbar_.insert(*space,-1);
		break;
	}

	case ToolbarBackend::MINIBUFFER:
		// Not supported yet.
		break;

	case ToolbarBackend::LAYOUTS: {
		layout_.reset(new GLayoutBox(owner_, toolbar_, func));
		break;
	}

	default: {
		Glib::ustring xpmName =
			Glib::locale_to_utf8(toolbarbackend.getIcon(func));
		Glib::ustring tip = Glib::locale_to_utf8(tooltip);
		Gtk::ToolButton * toolbutton;
		if (xpmName.size() == 0) {
			toolbutton = Gtk::manage(new Gtk::ToolButton);
		} else {
			Gtk::Image * image = Gtk::manage(new Gtk::Image(xpmName));
			image->show();
			toolbutton = Gtk::manage(new Gtk::ToolButton(*image));
		}
		// This code is putting a function reference into the GObject data field
		// named gToolData.  That's how we know how to update the status of the
		// toolitem later.
		toolbutton->set_data(gToolData,
			reinterpret_cast<void*>(&const_cast<FuncRequest &>(func)));
		tooltips_.set_tip(*toolbutton, tip, tip);
		toolbutton->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
			&GToolbar::clicked), FuncRequest(func)));
		toolbar_.insert(*toolbutton,-1);
		break;
	}

	}
}


void GToolbar::clicked(FuncRequest func)
{
	owner_.getLyXFunc().dispatch(func, true);
}


void GToolbar::hide(bool)
{
	toolbar_.hide();
}


void GToolbar::show(bool)
{
	toolbar_.show();
}


void GToolbar::update()
{
	int const items = toolbar_.get_n_items();

	for (int i = 0; i < items; ++i) {
		Gtk::ToolItem * item = toolbar_.get_nth_item(i);

		FuncRequest const * func = reinterpret_cast<FuncRequest *>(
			item->get_data(gToolData));
		if (func) {
			FuncStatus const status = owner_.getLyXFunc().getStatus(*func);
			item->set_sensitive(status.enabled());
		}
	}
}

} // namespace frontend
} // namespace lyx
