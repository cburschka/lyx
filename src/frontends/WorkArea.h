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

#include "frontends/GuiCursor.h"

#include "frontends/key_state.h"
#include "frontends/LyXKeySym.h"

class BufferView;

namespace lyx {
namespace frontend {

class Painter;

/**
 * The work area class represents the widget that provides the
 * view onto a document. It is owned by the BufferView, and
 * is responsible for handing events back to its owning BufferView.
 * It works in concert with the BaseScreen class to update the
 * widget view of a document.
 */
class WorkArea {
public:
	WorkArea(BufferView * buffer_view = 0);

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

	///
	void processKeySym(LyXKeySymPtr key, key_modifier::state state);

	/// grey out (no buffer)
	void greyOut();

	/// paint the cursor and store the background
	virtual void showCursor(int x, int y, int h, CursorShape shape) = 0;

	/// hide the cursor
	virtual void removeCursor() = 0;

protected:
	/// cause the display of the given area of the work area
	virtual void expose(int x, int y, int w, int h) = 0;

	///
	BufferView * buffer_view_;

private:
	///
	void checkAndGreyOut();

	///
	bool greyed_out_;
};

} // namespace frontend
} // namespace lyx

#endif // BASE_WORKAREA_H
