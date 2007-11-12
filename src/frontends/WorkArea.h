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

#include "frontends/KeyModifier.h"

namespace lyx {

class BufferView;
class KeySymbol;

namespace frontend {

class LyXView;
class Painter;

/**
 * The work area class represents the widget that provides the
 * view onto a document. It is owned by the BufferView, and
 * is responsible for handing events back to its owning BufferView.
 * It works in concert with the BaseScreen class to update the
 * widget view of a document.
 */
class WorkArea
{
public:
	///
	WorkArea() {}

	virtual ~WorkArea();

	///
	virtual void setLyXView(LyXView & lv) = 0;

	///
	virtual BufferView & bufferView() = 0;
	///
	virtual BufferView const & bufferView() const = 0;

	/// return true if has the keyboard input focus.
	virtual bool hasFocus() const = 0;

	/// return true if has this WorkArea is visible.
	virtual bool isVisible() const = 0;

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
	virtual void redraw() = 0;
	///
	virtual void stopBlinkingCursor() = 0;
	virtual void startBlinkingCursor() = 0;

	/// Process Key pressed event.
	/// This needs to be public because it is accessed externally by GuiView.
	virtual void processKeySym(KeySymbol const & key, KeyModifier mod) = 0;

	/// close this work area.
	/// Slot for Buffer::closing signal.
	virtual void close() = 0;
	/// This function is called when the buffer readonly status change.
	virtual void setReadOnly(bool) = 0;

	/// Update window titles of all users.
	virtual void updateWindowTitle() = 0;
};

} // namespace frontend
} // namespace lyx

#endif // BASE_WORKAREA_H
