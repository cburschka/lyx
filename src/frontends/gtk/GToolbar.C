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

char const * gToolData = "tool_data";

} // namespace anon


GLayoutBox::GLayoutBox(LyXView & owner,
		       Gtk::Toolbar & toolbar,
		       FuncRequest const & func)
	: owner_(owner),
	  internal_(false)
{
	combo_.signal_changed().connect(
		sigc::mem_fun(*this,&GLayoutBox::selected));

	model_ = Gtk::ListStore::create(cols_);
	combo_.set_model(model_);
	Gtk::CellRendererText * cell = Gtk::manage(new Gtk::CellRendererText);
	combo_.pack_start(*cell, true);
	combo_.add_attribute(*cell,"text",0);
	combo_.set_wrap_width(2);
	// Initially there's nothing in the liststore, so set the size
	// to avoid it jumping too much when the user does something that
	// causes the first update()
	combo_.set_size_request(130,-1);


	combo_.set_data(
		gToolData,
		reinterpret_cast<void*>(&const_cast<FuncRequest &>(func)));

	combo_.show();

	Gtk::ToolItem * toolitem = Gtk::manage(new Gtk::ToolItem);
	toolitem->add(combo_);
	toolbar.append(*toolitem);
}

void GLayoutBox::set(string const & layout)
{
	LyXTextClass const & tc = getTextClass(owner_);
	string const target = tc[layout]->name();

	internal_ = true;
	Gtk::TreeModel::iterator it = model_->children().begin();
	Gtk::TreeModel::iterator end = model_->children().end();
	for (; it != end; ++it) {
		if ((*it)[cols_.name] == target){
			combo_.set_active(it);
			internal_ = false;
			return;
		}
	}
	internal_ = false;

	lyxerr << "ERROR (GLayoutBox::set): layout not found! name: "
	       << target << std::endl;
}


void GLayoutBox::update()
{
	clear();

	LyXTextClass const & tc = getTextClass(owner_);

	LyXTextClass::const_iterator it = tc.begin();
	LyXTextClass::const_iterator const end = tc.end();

	internal_ = true;
	for (; it != end; ++it)
		if ((*it)->obsoleted_by().empty()) {
			Gtk::TreeModel::iterator iter = model_->append();
			Gtk::TreeModel::Row row = *iter;
			row[cols_.name] = Glib::locale_to_utf8((*it)->name());
		}
	internal_ = false;

	// now that we've loaded something into the combobox, forget
	// the initial fixed size and let GTK decide.
	combo_.set_size_request(-1,-1);
}


void GLayoutBox::clear()
{
	internal_ = true;
	model_->clear();
	internal_ = false;
}


void GLayoutBox::open()
{
	combo_.popup();
}


void GLayoutBox::setEnabled(bool sensitive)
{
	combo_.set_sensitive(sensitive);
}


void GLayoutBox::selected()
{
	if (internal_)
		return;

	Glib::ustring layoutGuiName = (*(combo_.get_active()))[cols_.name];

	// we get two signal, one of it is empty and useless
	if (layoutGuiName.empty())
		return;
	LyXTextClass const & tc = getTextClass(owner_);

	LyXTextClass::const_iterator it = tc.begin();
	LyXTextClass::const_iterator const end = tc.end();
	for (; it != end; ++it) {
		string const & name = (*it)->name();
		if (name == layoutGuiName) {
			FunctRequest const func(LFUN_LAYOUT, name, 
						FuncRequest::UI);
			owner_.getLyXFunc().dispatch(func);
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

	toolbar_.set_tooltips(true);
}

void GToolbar::add(FuncRequest const & func, string const & tooltip)
{
	switch (func.action) {
	case ToolbarBackend::SEPARATOR: {
		Gtk::SeparatorToolItem * space =
			Gtk::manage(new Gtk::SeparatorToolItem);
		toolbar_.append(*space);
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

		toolbutton->set_tooltip(*toolbar_.get_tooltips_object(),tip);
		/*toolbar_.get_tooltips_object()->set_tip(*toolbutton, tip);*/

		toolbutton->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
			&GToolbar::clicked), FuncRequest(func)));
		toolbar_.append(*toolbutton);
		break;
	}

	}
}


void GToolbar::clicked(FuncRequest func)
{
	owner_.getLyXFunc().dispatch(func);
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
