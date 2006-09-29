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

#include "bufferview_funcs.h"

#include "frontends/LyXView.h"

#include <gtkmm.h>

#include <map>

namespace lyx {
namespace frontend {

class GMiniBuffer;

class GView : public LyXView, public Gtk::Window {
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

	/// initialize the object
	virtual void init();

	/// FIXME: not implemented!
	virtual void setGeometry(
		unsigned int width,
		unsigned int height,
		int posx, int posy,
		bool maximize);

	Gtk::Box & getBox(Position pos);

	virtual void prohibitInput() const;
	virtual void allowInput() const;
	virtual void message(lyx::docstring const &);

	bool on_delete_event(GdkEventAny * event);
	void focusWorkArea() { workArea_->grab_focus(); }
	void setGWorkArea(Gtk::Widget * w) { workArea_ = w; }

	/// show busy cursor
	virtual void busy(bool) const;

	///
	Toolbars::ToolbarPtr makeToolbar(ToolbarBackend::Toolbar const & tbb);

	/// clear any temporary message and replace with current status
	virtual void clearMessage();

	// returns true if this view has the focus.
	virtual bool hasFocus() const;
	///
	void updateStatusBar();

private:
	bool onFocusIn(GdkEventFocus * event);
	virtual void setWindowTitle(lyx::docstring const & t, lyx::docstring const & it);

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

} // namespace frontend
} // namespace lyx

#endif
