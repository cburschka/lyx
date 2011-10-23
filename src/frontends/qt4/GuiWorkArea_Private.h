// -*- C++ -*-
/**
 * \file GuiWorkArea_Private.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef WORKAREA_PRIVATE_H
#define WORKAREA_PRIVATE_H

// Comment out to use QImage backend instead of QPixmap. This won't have any
// effect on Windows, MacOSX and most X11 environment when running locally.
// When running remotely on X11, this may have a big performance penalty.
//#define USE_QIMAGE

#include "FuncRequest.h"
#include "qt_helpers.h"

#include "support/docstring.h"
#include "support/Timeout.h"

#include <QAbstractScrollArea>
#include <QMouseEvent>
#ifdef USE_QIMAGE
#include <QImage>
#else
#include <QPixmap>
#endif
#include <QTimer>

class QContextMenuEvent;
class QDragEnterEvent;
class QDropEvent;
class QKeyEvent;
class QPaintEvent;
class QResizeEvent;
class QToolButton;
class QWheelEvent;
class QWidget;

#ifdef CursorShape
#undef CursorShape
#endif

namespace lyx {

class Buffer;

namespace frontend {

class GuiCompleter;
class GuiView;
class GuiWorkArea;

/// for emulating triple click
class DoubleClick {
public:
	///
	DoubleClick() : state(Qt::NoButton), active(false) {}
	///
	DoubleClick(QMouseEvent * e) : state(e->button()), active(true) {}
	///
	bool operator==(QMouseEvent const & e) { return state == e.button(); }
	///
public:
	///
	Qt::MouseButton state;
	///
	bool active;
};

/** Qt only emits mouse events when the mouse is being moved, but
 *  we want to generate 'pseudo' mouse events when the mouse button is
 *  pressed and the mouse cursor is below the bottom, or above the top
 *  of the work area. In this way, we'll be able to continue scrolling
 *  (and selecting) the text.
 *
 *  This class stores all the parameters needed to make this happen.
 */
class SyntheticMouseEvent
{
public:
	SyntheticMouseEvent();

	FuncRequest cmd;
	Timeout timeout;
	bool restart_timeout;
};


/**
 * Implementation of the work area (buffer view GUI)
*/
class CursorWidget;

struct GuiWorkArea::Private
{
	Private(GuiWorkArea *);

	/// update the passed area.
	void update(int x, int y, int w, int h);
	///
	void updateScreen();
	///
	void resizeBufferView();

	/// paint the cursor and store the background
	virtual void showCursor(int x, int y, int h,
		bool l_shape, bool rtl, bool completable);

	/// hide the cursor
	virtual void removeCursor();
	///
	void dispatch(FuncRequest const & cmd0, KeyModifier = NoModifier);
	/// hide the visible cursor, if it is visible
	void hideCursor();
	/// show the cursor if it is not visible
	void showCursor();
	///
	void updateScrollbar();
	/// Change the cursor when the mouse hovers over a clickable inset
	void updateCursorShape();
	///
	void setCursorShape(Qt::CursorShape shape);

#ifdef USE_QIMAGE
	void resetScreen()
	{
		screen_ = QImage(p->viewport()->width(), p->viewport()->height(),
			QImage::Format_ARGB32_Premultiplied);
	}

	QImage screen_;
#else
	void resetScreen()
	{
		screen_ = QPixmap(p->viewport()->width(), p->viewport()->height());
	}

	QPixmap screen_;
#endif
	///
	GuiWorkArea * p;

	///
	BufferView * buffer_view_;
	/// Read only Buffer status cache.
	bool read_only_;
	///
	GuiView * lyx_view_;
	/// is the cursor currently displayed
	bool cursor_visible_;

	///
	QTimer cursor_timeout_;
	///
	SyntheticMouseEvent synthetic_mouse_event_;
	///
	DoubleClick dc_event_;

	///
	CursorWidget * cursor_;
	///
	bool need_resize_;
	///
	bool schedule_redraw_;
	///
	int preedit_lines_;

	///
	GuiCompleter * completer_;

	/// Special mode in which Esc and Enter (with or without Shift)
	/// are ignored
	bool dialog_mode_;
	/// store the name of the context menu when the mouse is
	/// pressed. This is used to get the correct context menu 
	/// when the menu is actually shown (after releasing on Windows)
	/// and after the DEPM has done its job.
	docstring context_menu_name_;
}; // GuiWorkArea

} // namespace frontend
} // namespace lyx

#endif // WORKAREA_H
