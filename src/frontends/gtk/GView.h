// -*- C++ -*-
/**
 * \file gtk/GView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GVIEW_H
#define GVIEW_H

#include "frontends/LyXView.h"
#include "bufferview_funcs.h"
#include <map>

class GMiniBuffer;

class GView : public LyXView, public Gtk::Window
{
public:
	enum Position {
		Top,
		Bottom,
		Left,
		Right,
		Center
	};

	GView();
	~GView();

	Gtk::Box & getBox(Position pos);

	virtual void prohibitInput() const;
	virtual void allowInput() const;
	virtual void message(std::string const &);

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
	virtual void setWindowTitle(std::string const & t, std::string const & it);
	static GView * view_;

	// The top-most box containing all other boxes.
	Gtk::VBox top_box_;

	// The physical store for the boxes making up the layout.
	typedef boost::shared_ptr<Gtk::Box> BoxPtr;
	typedef std::vector<BoxPtr> BoxStore;
	BoxStore box_store_;

	// Accessors to the various Boxes.
	std::map<Position, BoxPtr> box_map_;

	boost::scoped_ptr<GMiniBuffer> minibuffer_;
	Gtk::Widget * workArea_;
};

#endif
