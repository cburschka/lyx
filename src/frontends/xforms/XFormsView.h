// -*- C++ -*-
/**
 * \file XFormsView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LyXView_H
#define LyXView_H

#include "LayoutEngine.h"
#include "forms_fwd.h"

#include "frontends/LyXView.h"
#include <X11/Xlib.h> // for Pixmap

#include <boost/signal.hpp>

#include <map>

namespace lyx {
namespace frontend {

class XMiniBuffer;

/**
 * XFormsView - xforms implementation of LyXView
 *
 * xforms-private implementation of the main LyX window.
 */
class XFormsView : public LyXView {
public:
	enum Position {
		Top,
		Bottom,
		Left,
		Right,
		Center
	};

	/// create a main window of the given dimensions
	XFormsView(int w, int h);

	~XFormsView();

	/// Accessor to the appropriate layout Box.
	Box & getBox(Position pos) const;

	/**
	 * show - display the top-level window
	 * @param xpos requested x position (or 0)
	 * @param xpos requested y position (or 0)
	 * @param title window title
	 */
	void show(int xpos, int ypos, std::string const & t = std::string("LyX"));

	/// get the xforms main form
	FL_FORM * getForm() const;
	/// redraw the main form.
	virtual void redraw();
	/// show busy cursor
	virtual void busy(bool) const;

	/// callback for close event from window manager
	static int atCloseMainFormCB(FL_FORM *, void *);

	/// display a status message
	virtual void message(std::string const & str);

	/// clear back to normal status message
	virtual void clearMessage();

	///
	void updateMetrics(bool resize_form = false);
	///
	boost::signal<void()> metricsUpdated;

	// returns true if this view has the focus.
	virtual bool hasFocus() const;

private:
	/**
	 * setWindowTitle - set title of window
	 * @param t main window title
	 * @param it iconified (short) title
	 */
	virtual void setWindowTitle(std::string const & t, std::string const & it);

	/// update the minibuffer state message
	void show_view_state();

	/// The top-most box of the layout engine containing all other boxes.
	Box window_;

	// Accessors to the various Boxes.
	std::map<Position, Box *> box_map_;

	/// the minibuffer
	boost::scoped_ptr<XMiniBuffer> minibuffer_;
	///
	boost::signals::connection view_state_con;
	///
	boost::signals::connection focus_con;
	///
	boost::signals::connection redraw_con;

	/// the main form.
	FL_FORM * form_;
	/// Passed to the window manager to give a pretty little symbol ;-)
	Pixmap icon_pixmap_;
	///
	Pixmap icon_mask_;
};

} // namespace frontend
} // namespace lyx

#endif
