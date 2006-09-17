// -*- C++ -*-
/**
 * \file WorkArea.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BASE_WORKAREA_H
#define BASE_WORKAREA_H

#include "frontends/key_state.h"
#include "frontends/LyXKeySym.h"
#include "frontends/Timeout.h"

#include "support/docstring.h"

#include <boost/signals/trackable.hpp>

// FIXME: defined in X.h, spuriously pulled in by Qt 3 headers
#undef CursorShape

class BufferView;
class FuncRequest;
class LyXView;


namespace lyx {
namespace frontend {

class Painter;

/// types of cursor in work area
enum CursorShape {
	/// normal I-beam
	BAR_SHAPE,
	/// L-shape for locked insets of a different language
	L_SHAPE,
	/// reverse L-shape for RTL text
	REVERSED_L_SHAPE
};

/**
 * The work area class represents the widget that provides the
 * view onto a document. It is owned by the BufferView, and
 * is responsible for handing events back to its owning BufferView.
 * It works in concert with the BaseScreen class to update the
 * widget view of a document.
 */
class WorkArea : public boost::signals::trackable {
public:
	WorkArea(LyXView & lyx_view);

	virtual ~WorkArea() {}

	void setBufferView(BufferView * buffer_view);

	///
	BufferView & bufferView();
	///
	BufferView const & bufferView() const;


	/// return the painter object for this work area
	virtual Painter & getPainter() = 0;

	/// return the width of the work area in pixels
	virtual int width() const = 0;

	/// return the height of the work area in pixels
	virtual int height() const = 0;

	/**
	 * Update the scrollbar.
	 * @param height the total document height in pixels
	 * @param pos the current position in the document, in pixels
	 * @param line_height the line-scroll amount, in pixels
	 */
	virtual void setScrollbarParams(int height, int pos, int line_height) = 0;

	/// redraw the screen, without using existing pixmap
	virtual void redraw();

	/// grey out (no buffer)
	void greyOut();

	/// FIXME: should be protected, public until the qt3 and gtk frontends are
	/// cleaned up.
	void processKeySym(LyXKeySymPtr key, key_modifier::state state);

protected:
	/// cause the display of the given area of the work area
	virtual void expose(int x, int y, int w, int h) = 0;

public:
	/// FIXME: This is public because of qt3 and gtk, should be protected
	void dispatch(FuncRequest const & cmd0);

	/// FIXME: This is public because of qt3 and gtk, should be protected
	void resizeBufferView();

	/// FIXME: This is public because of qt3 and gtk, should be protected
	void scrollBufferView(int position);

protected:
	/// hide the visible cursor, if it is visible
	void hideCursor();

	/// show the cursor if it is not visible
	void showCursor();

	/// toggle the cursor's visibility
	void toggleCursor();

	/// hide the cursor
	virtual void removeCursor() = 0;

	/// paint the cursor and store the background
	virtual void showCursor(int x, int y, int h, CursorShape shape) = 0;

	///
	BufferView * buffer_view_;

	///
	LyXView & lyx_view_;

private:
	///
	void updateScrollbar();
	///
	void checkAndGreyOut();
	///
	void displayMessage(lyx::docstring const &);
	/// buffer messages signal connection
	boost::signals::connection message_connection_;

	///
	bool greyed_out_;

	/// is the cursor currently displayed
	bool cursor_visible_;

	///
	Timeout cursor_timeout_;
};

} // namespace frontend
} // namespace lyx

#endif // BASE_WORKAREA_H
