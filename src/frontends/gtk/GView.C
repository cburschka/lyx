/**
 * \file GView.C
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
#include "MenuBackend.h"
#include "support/filetools.h"
#include "GMenubar.h"
#include "GToolbar.h"
#include "BufferView.h"
#include "XWorkArea.h"
#include "lyx_cb.h"
#include "GMiniBuffer.h"
#include "lyxfunc.h"
#include <boost/bind.hpp>

using std::string;


BufferView * current_view;

GView * GView::view_ = 0;


GView::GView()
{
	view_ = this;
	vbox_.reset(new Gtk::VBox);
	add(*vbox_.get());
	menubar_.reset(new GMenubar(this, menubackend));
	toolbar_.reset(new GToolbar(this, 0, 0));
	toolbar_->init();
	bufferview_.reset(new BufferView(this, 0, 0, 300, 300));
	::current_view = bufferview_.get();
	minibuffer_.reset(new GMiniBuffer(this, *controlcommand_));
	vbox_->show();
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
{
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
	message(getLyXFunc().view_status_message());
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
		const_cast<GView*>(this)->get_window()->set_cursor(cursor);
		const_cast<GView*>(this)->set_sensitive(false);
	} else {
		const_cast<GView*>(this)->get_window()->set_cursor();
		const_cast<GView*>(this)->set_sensitive(true);
	}
}


void GView::clearMessage()
{
	message(getLyXFunc().view_status_message());
}
