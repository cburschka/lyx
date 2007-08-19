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
#include "frontends/KeySymbol.h"

#include "support/Timeout.h"
#include "support/docstring.h"

#include <boost/signals/trackable.hpp>

#undef CursorShape

namespace lyx {

class BufferView;
class FuncRequest;

namespace frontend {

class LyXView;
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
	WorkArea(int id, LyXView & lyx_view);

	virtual ~WorkArea() {}

	int const id() const { return id_; }

	void setBufferView(BufferView * buffer_view);

	///
	BufferView & bufferView();
	///
	BufferView const & bufferView() const;

	/// \return true if has the keyboard input focus.
	virtual bool hasFocus() const = 0;

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

	///
	virtual void scheduleRedraw() = 0;

	/// redraw the screen, without using existing pixmap
	virtual void redraw();
	///
	void stopBlinkingCursor();
	void startBlinkingCursor();

	/// Process Key pressed event.
	/// This needs to be public because it is accessed externally by GuiView.
	void processKeySym(KeySymbolPtr key, key_modifier::state state);
protected:
	/// cause the display of the given area of the work area
	virtual void expose(int x, int y, int w, int h) = 0;
	///
	void dispatch(FuncRequest const & cmd0,
		key_modifier::state = key_modifier::none);
	///
	void resizeBufferView();
	///
	void scrollBufferView(int position);
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
	void updateScrollbar();

	///
	BufferView * buffer_view_;

	///
	LyXView & lyx_view_;
	///
	bool greyed_out_;

private:
	///
	int id_;
	///
	void displayMessage(docstring const &);
	/// buffer messages signal connection
	boost::signals::connection message_connection_;

	/// is the cursor currently displayed
	bool cursor_visible_;

	///
	Timeout cursor_timeout_;
};

} // namespace frontend
} // namespace lyx

#endif // BASE_WORKAREA_H
