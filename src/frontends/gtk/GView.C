/**
 * \file gtk/GView.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <gtkmm.h>

#include "GView.h"
#include "GMenubar.h"
#include "GMiniBuffer.h"
#include "GToolbar.h"

#include "BufferView.h"
#include "lyx_cb.h"
#include "lyxfunc.h"
#include "MenuBackend.h"

#include "support/filetools.h"

#include <boost/bind.hpp>

#include <vector>

using std::string;


namespace {

void add_el(Gtk::Box::BoxList & list, Gtk::Box & box, bool shrink)
{
	Gtk::PackOptions const packing =
		shrink ? Gtk::PACK_SHRINK : Gtk::PACK_EXPAND_WIDGET;
	list.push_back(Gtk::Box_Helpers::Element(box, packing));
}

} // namespace anon


GView::GView()
{
	// The physical store for the boxes making up the layout.
	box_store_.push_back(boost::shared_ptr<Gtk::Box>(new Gtk::VBox));
	box_store_.push_back(boost::shared_ptr<Gtk::Box>(new Gtk::HBox));
	box_store_.push_back(boost::shared_ptr<Gtk::Box>(new Gtk::VBox));
	box_store_.push_back(boost::shared_ptr<Gtk::Box>(new Gtk::HBox));
	box_store_.push_back(boost::shared_ptr<Gtk::Box>(new Gtk::HBox));
	box_store_.push_back(boost::shared_ptr<Gtk::Box>(new Gtk::HBox));

	// Lay them out correctly.
	add(top_box_);

 	Gtk::Box::BoxList & layout1 = top_box_.children();
	add_el(layout1, *box_store_[0], true);
	add_el(layout1, *box_store_[1], true);
	add_el(layout1, *box_store_[2], true);

 	Gtk::Box::BoxList & layout2 = box_store_[1]->children();
	add_el(layout2, *box_store_[3], true);
	add_el(layout2, *box_store_[4], false);
	add_el(layout2, *box_store_[5], true);

	// Define accessors to the various Boxes.
	box_map_[Top]    = box_store_[0];
	box_map_[Bottom] = box_store_[2];
	box_map_[Left]   = box_store_[3];
	box_map_[Center] = box_store_[4];
	box_map_[Right]  = box_store_[5];

	// Make all Boxes visible.
	top_box_.show();
	BoxStore::iterator it = box_store_.begin();
	BoxStore::iterator const end = box_store_.end();
	for (; it != end; ++it)
		(*it)->show();

	// Define the components making up the window.
	menubar_.reset(new GMenubar(this, menubackend));
	toolbar_.reset(new GToolbar(this, 0, 0));
	toolbar_->init();
	bufferview_.reset(new BufferView(this, 0, 0, 300, 300));
	minibuffer_.reset(new GMiniBuffer(this, *controlcommand_));

	focus_command_buffer.connect(
		boost::bind(&GMiniBuffer::editMode, minibuffer_.get()));
	view_state_changed.connect(boost::bind(&GView::showViewState, this));
	signal_focus_in_event().connect(SigC::slot(*this, &GView::onFocusIn));
	set_default_size(500, 550);
	// Make sure the buttons are disabled if needed.
	updateToolbar();
	string const iconName =
		lyx::support::LibFileSearch("images", "lyx", "xpm");
	if (!iconName.empty())
		set_icon_from_file(iconName);
}


GView::~GView()
{}


Gtk::Box & GView::getBox(Position pos)
{
	return *box_map_[pos];
}


bool GView::on_delete_event(GdkEventAny * /*event*/)
{
	QuitLyX();
	return true;
}


bool GView::onFocusIn(GdkEventFocus * /*event*/)
{
	workArea_->grab_focus();
	return true;
}


void GView::prohibitInput() const
{
	view()->hideCursor();
	const_cast<GView*>(this)->set_sensitive(false);
}


void GView::allowInput() const
{
	const_cast<GView*>(this)->set_sensitive(true);
}


void GView::message(string const & msg)
{
	minibuffer_->message(msg);
}


void GView::showViewState()
{
	message(getLyXFunc().viewStatusMessage());
}


void GView::setWindowTitle(string const & t, string const & /*it*/)
{
	set_title(Glib::locale_to_utf8(t));
}


void GView::busy(bool yes) const
{
	if (yes ) {
		view()->hideCursor();
		Gdk::Cursor cursor(Gdk::WATCH);
		const_cast<GView *>(this)->get_window()->set_cursor(cursor);
		const_cast<GView *>(this)->set_sensitive(false);
	} else {
		const_cast<GView *>(this)->get_window()->set_cursor();
		const_cast<GView *>(this)->set_sensitive(true);
	}
}


void GView::clearMessage()
{
	message(getLyXFunc().viewStatusMessage());
}
