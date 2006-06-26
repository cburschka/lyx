// -*- C++ -*-
/**
 * \file GuiCursor.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

// X11 use a define called CursorShape, and we really want to use
// that name our selves. Therefore we do something similar to what is done
// in kde/fixx11h.h:
namespace X {
#ifdef CursorShape
#ifndef FIXX11H_CursorShape
#define FIXX11H_CursorShape
int const XCursorShape = CursorShape;
#undef CursorShape
int const CursorShape = CursorShape;
#endif
#undef CursorShape
#endif

} // namespace X

#ifndef GUI_CURSOR_H
#define GUI_CURSOR_H


class LyXText;
class CursorSlice;
class BufferView;
class ViewMetricsInfo;

namespace lyx {
namespace frontend {

class WorkArea;

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
 * GuiCursor - document rendering management
 *
 * The blinking cursor is handled here.
 */
class GuiCursor {
public:
	GuiCursor();

	virtual ~GuiCursor();

	void connect(WorkArea * work_area);

	/// hide the visible cursor, if it is visible
	void hide();

	/// show the cursor if it is not visible
	void show(BufferView & bv);

	/// toggle the cursor's visibility
	void toggle(BufferView & bv);

	/// set cursor_visible_ to false in prep for re-display
	void prepare();

private:
	/// is the cursor currently displayed
	bool cursor_visible_;

	WorkArea * work_area_;
};

} // namespace frontend
} // namespace lyx

#endif // GUI_CURSOR_H
