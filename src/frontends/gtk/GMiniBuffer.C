/**
 * \file GMiniBuffer.C
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
#include "GMiniBuffer.h"
#include "debug.h"
#include "bufferview_funcs.h"
#include <boost/bind.hpp>
#include <vector>
#include "frontends/controllers/ControlCommandBuffer.h"


GMiniBuffer::GMiniBuffer(GView * view, ControlCommandBuffer & control) :
	controller_(control), view_(view)
{
	listCols_.add(listCol_);
	listStore_ = Gtk::ListStore::create(listCols_);
	listView_.set_model(listStore_);
	listView_.append_column("Completions", listCol_);
	listView_.signal_key_press_event().connect(
		SigC::slot(*this, &GMiniBuffer::onListKeyPress));
	listView_.signal_focus_in_event().connect(
		SigC::slot(*this, &GMiniBuffer::onListFocusIn));
	listView_.signal_focus_out_event().connect(
		SigC::slot(*this, &GMiniBuffer::onFocusOut));
	listSel_ = listView_.get_selection();
	listSel_->signal_changed().connect(
		SigC::slot(*this, &GMiniBuffer::onSelected));
	listView_.show();
	scrolledWindow_.set_policy(Gtk::POLICY_AUTOMATIC,
				   Gtk::POLICY_AUTOMATIC);
	scrolledWindow_.set_size_request(300, 150);
	scrolledWindow_.add(listView_);
	view_->getVBox().children().push_back(
		Gtk::Box_Helpers::Element(scrolledWindow_,
					  Gtk::PACK_SHRINK));
	entry_.signal_key_press_event().connect(
		SigC::slot(*this, &GMiniBuffer::onKeyPress));
	entry_.signal_focus_in_event().connect(
		SigC::slot(*this, &GMiniBuffer::onFocusIn));
	entry_.signal_focus_out_event().connect(
		SigC::slot(*this, &GMiniBuffer::onFocusOut));
	entry_.signal_activate().connect(
		SigC::slot(*this, &GMiniBuffer::onCommit));
	entry_.show();
	view_->getVBox().children().push_back(
		Gtk::Box_Helpers::Element(entry_,
					  Gtk::PACK_SHRINK));
	infoTimer_.reset(new Timeout(1500));
	idleTimer_.reset(new Timeout(6000));
	focusTimer_.reset(new Timeout(50));
	infoCon_ = infoTimer_->timeout.connect(
		boost::bind(&GMiniBuffer::infoTimeout, this));
	idleCon_ = idleTimer_->timeout.connect(
		boost::bind(&GMiniBuffer::idleTimeout, this));
	focusTimer_->timeout.connect(
		boost::bind(&GMiniBuffer::focusTimeout, this));
	idleTimer_->start();
	messageMode();
}


GMiniBuffer::~GMiniBuffer()
{
}


void GMiniBuffer::message(string const & str)
{
	if (!isEditMode())
		setInput(Glib::locale_to_utf8(str));
}


void GMiniBuffer::showInfo(Glib::ustring const & info, bool append)
{
	storedInput_ = entry_.get_text();
	entry_.set_editable(false);
	infoShown_ = true;
	if (append)
		setInput(storedInput_ + ' ' + info);
	else
		setInput(info);
	infoTimer_->start();
}


void GMiniBuffer::onSelected()
{
	if (!listSel_->count_selected_rows())
		return;
	Gtk::TreeModel::iterator it = listSel_->get_selected();
	Glib::ustring sel = (*it)[listCol_];
	setInput(sel + ' ');
}


void GMiniBuffer::onCommit()
{
	controller_.dispatch(Glib::locale_from_utf8(entry_.get_text()));
	messageMode();
}


bool GMiniBuffer::onListFocusIn(GdkEventFocus * /*event*/)
{
	if (focusTimer_->running())
		focusTimer_->stop();
	if (infoShown_) {
		infoTimer_->stop();
		infoTimeout();
	}
	return false;
}


bool GMiniBuffer::onFocusIn(GdkEventFocus * /*event*/)
{
	if (infoShown_) {
		infoTimer_->stop();
		infoTimeout();
	}
	if (focusTimer_->running()) {
		focusTimer_->stop();
		return false;
	}
	setInput("");
	idleTimer_->stop();
	return false;
}


bool GMiniBuffer::onFocusOut(GdkEventFocus * /*event*/)
{
	focusTimer_->start();
	return false;
}


void GMiniBuffer::focusTimeout()
{
	if (infoShown_) {
		infoTimer_->stop();
		infoTimeout();
	}
	focusTimer_->stop();
	setInput("");
	idleTimer_->start();
	scrolledWindow_.hide();
}


bool GMiniBuffer::onListKeyPress(GdkEventKey * event)
{
	if (infoShown_) {
		infoTimer_->stop();
		infoTimeout();
	}
	switch (event->keyval) {
	case GDK_Escape:
		messageMode();
		break;
	case GDK_Tab:
		entry_.grab_focus();
		setInput(entry_.get_text() + ' ');
		break;
	}
	return true;
}


bool GMiniBuffer::onKeyPress(GdkEventKey * event)
{
	if (infoShown_) {
		infoTimer_->stop();
		infoTimeout();
	}
	switch (event->keyval) {
	case GDK_Down:
	{
		Glib::ustring const h = 
			Glib::locale_to_utf8(controller_.historyDown());
		if (h.empty())
			showInfo("[End of history]", false);
		else
			setInput(h);
		break;
	}
	case GDK_Up:
	{
		Glib::ustring const h = 
			Glib::locale_to_utf8(controller_.historyUp());
		if (h.empty())
			showInfo("[Beginning of history]", false);
		else
			setInput(h);
		break;
	}
	case GDK_Escape:
		messageMode();
		break;
	case GDK_Tab:
	{
		Glib::ustring new_input, input;
		string new_input_locale;
		input = entry_.get_text();
		std::vector<string> comp = 
			controller_.completions(Glib::locale_from_utf8(input),
						new_input_locale);
		new_input = Glib::locale_to_utf8(new_input_locale);
		if (comp.empty() && new_input == input) {
			showInfo("[no match]");
			break;
		}

		if (comp.empty()) {
			setInput(new_input + ' ');
			showInfo("[only completion]");
			break;
		}
		setInput(new_input);
		listStore_->clear();
		std::vector<string>::iterator it;
		for (it = comp.begin(); it != comp.end(); ++it)
			(*listStore_->append())[listCol_] =
				Glib::locale_to_utf8(*it);
		scrolledWindow_.show();
		break;
	}
	}
	return true;
}


bool GMiniBuffer::isEditMode() const
{
	return entry_.has_focus() || listView_.has_focus();
}


void GMiniBuffer::infoTimeout()
{
	infoShown_ = false;
	setInput(storedInput_);
	entry_.set_editable(true);
}


void GMiniBuffer::idleTimeout()
{
	setInput(Glib::locale_to_utf8(controller_.getCurrentState()));
}


void GMiniBuffer::editMode()
{
	entry_.grab_focus();
}


void GMiniBuffer::messageMode()
{
	view_->focusWorkArea();
}


void GMiniBuffer::setInput(Glib::ustring const & input)
{
	entry_.set_text(input);
	entry_.set_position(-1);
}
