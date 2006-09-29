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

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GView.h"
#include "GMenubar.h"
#include "GToolbar.h"
#include "GMiniBuffer.h"

#include "BufferView.h"
#include "lyx_cb.h"
#include "lyxrc.h"
#include "lyx_main.h"
#include "session.h"
#include "lyxfunc.h"
#include "MenuBackend.h"
#include "funcrequest.h"

#include "frontends/Toolbars.h"
#include "frontends/WorkArea.h"

#include "support/filetools.h"
#include "support/convert.h"

#include <boost/bind.hpp>

#include <vector>

using std::string;

namespace lyx {
namespace frontend {

namespace {

void add_el(Gtk::Box::BoxList & list, Gtk::Box & box, bool shrink)
{
	Gtk::PackOptions const packing =
		shrink ? Gtk::PACK_SHRINK : Gtk::PACK_EXPAND_WIDGET;
	list.push_back(Gtk::Box_Helpers::Element(box, packing));
}

} // namespace anon


GView::GView() : LyXView()
{
	// The physical store for the boxes making up the layout.
	box_store_.push_back(BoxPtr(new Gtk::VBox));
	box_store_.push_back(BoxPtr(new Gtk::HBox));
	box_store_.push_back(BoxPtr(new Gtk::VBox));
	box_store_.push_back(BoxPtr(new Gtk::HBox));
	box_store_.push_back(BoxPtr(new Gtk::HBox));
	box_store_.push_back(BoxPtr(new Gtk::HBox));

	// Lay them out correctly.
	add(top_box_);

	Gtk::Box::BoxList & layout1 = top_box_.children();
	add_el(layout1, *box_store_[0], true);
	add_el(layout1, *box_store_[1], false);
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
	top_box_.show_all();

	// Define the components making up the window.
	menubar_.reset(new GMenubar(this, menubackend));
	getToolbars().init();
	minibuffer_.reset(new GMiniBuffer(this, *controlcommand_));

	focus_command_buffer.connect(
		boost::bind(&GMiniBuffer::editMode, minibuffer_.get()));
	signal_focus_in_event().connect(sigc::mem_fun(*this, &GView::onFocusIn));
	//
	int width = 750;
	int height = 550;
	// first try lyxrc
	if (lyxrc.geometry_width != 0 && lyxrc.geometry_height != 0 ) {
		width = lyxrc.geometry_width;
		height = lyxrc.geometry_height;
	}
	// if lyxrc returns (0,0), then use session info
	else {
		string val = LyX::ref().session().loadSessionInfo("WindowWidth");
		if (val != "")
			width = convert<unsigned int>(val);
		val = LyX::ref().session().loadSessionInfo("WindowHeight");
		if (val != "")
			height = convert<unsigned int>(val);
	}
	set_default_size(width, height);
	// Make sure the buttons are disabled if needed.
	//updateToolbars();
	string const iconName =
		support::libFileSearch("images", "lyx", "xpm");
	if (!iconName.empty())
		set_icon_from_file(iconName);
}


GView::~GView()
{}


void GView::init()
{
	updateLayoutChoice();
	updateMenubar();
}


void GView::setGeometry(unsigned int width,
						 unsigned int height,
						 int posx, int posy,
						 bool maximize)
{
// FIXME: do something here...
}


Gtk::Box & GView::getBox(Position pos)
{
	return *box_map_[pos];
}


bool GView::on_delete_event(GdkEventAny * /*event*/)
{
	// save windows size and position
	Gtk::Requisition req = workArea_->size_request();
	LyX::ref().session().saveSessionInfo("WindowWidth", convert<string>(req.width));
	LyX::ref().session().saveSessionInfo("WindowHeight", convert<string>(req.height));
	// trigger LFUN_LYX_QUIT instead of quit directly
	// since LFUN_LYX_QUIT may have more cleanup stuff
	//
	getLyXFunc().dispatch(FuncRequest(LFUN_LYX_QUIT));
	return true;
}


bool GView::onFocusIn(GdkEventFocus * /*event*/)
{
	workArea_->grab_focus();
	return true;
}


void GView::prohibitInput() const
{
	// FIXME: Why is prohibitInput const?
	// FIXME: hideCursor is protected
	//const_cast<GView*>(this)->workArea()->hideCursor();
	const_cast<GView*>(this)->set_sensitive(false);
}


void GView::allowInput() const
{
	const_cast<GView*>(this)->set_sensitive(true);
}


void GView::message(docstring const & msg)
{
	minibuffer_->message(lyx::to_utf8(msg));
}


void GView::updateStatusBar()
{
	message(lyx::from_utf8(getLyXFunc().viewStatusMessage()));
}


void GView::setWindowTitle(docstring const & t, docstring const & /*it*/)
{
	set_title(lyx::to_utf8(t));
}


void GView::busy(bool yes) const
{
	// FIXME: Why is busy const?
	if (yes) {
		// FIXME: hideCursor is protected
		//const_cast<GView*>(this)->workArea()->hideCursor();
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
	message(lyx::from_utf8(getLyXFunc().viewStatusMessage()));
}


bool GView::hasFocus() const
{
	// No real implementation needed for now
	return true;
}


Toolbars::ToolbarPtr GView::makeToolbar(ToolbarBackend::Toolbar const & tbb)
{
	return make_toolbar(tbb, *this);
}

} // namespace frontend
} // namespace lyx
