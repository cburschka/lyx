// -*- C++ -*-
/**
 * \file GView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef GVIEW_H
#define GVIEW_H

#include "frontends/LyXView.h"
#include "bufferview_funcs.h"
#include <memory>
class GMiniBuffer;

class GView : public LyXView, public Gtk::Window
{
public:
	virtual ~GView();

	virtual void prohibitInput() const;
	virtual void allowInput() const;
	virtual void message(string const &);
	Gtk::VBox & getVBox() { return *vbox_.get(); }
	GView();
	bool on_delete_event(GdkEventAny * event);
	void focusWorkArea() { workArea_->grab_focus(); }
	void setGWorkArea(Gtk::Widget * w) { workArea_ = w; }
	static GView * instance() { return view_; }
	/// show busy cursor
	virtual void busy(bool) const;
	/// clear any temporary message and replace with current status
	virtual void clearMessage();
private:
	void showViewState();
	bool onFocusIn(GdkEventFocus * event);
	virtual void setWindowTitle(string const & t, string const & it);
	static GView * view_;
	std::auto_ptr<Gtk::VBox> vbox_;
	boost::scoped_ptr<GMiniBuffer> minibuffer_;
	Gtk::Widget * workArea_;
};

#endif
