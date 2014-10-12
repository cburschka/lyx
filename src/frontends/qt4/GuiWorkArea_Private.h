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

#include "FuncRequest.h"
#include "LyXRC.h"

#include "support/Timeout.h"

#include <QMouseEvent>
#include <QImage>
#include <QPixmap>
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
	void showCursor(int x, int y, int h,
		bool l_shape, bool rtl, bool completable);

	/// hide the cursor
	void removeCursor();
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

	bool needResize() const {
		return need_resize_ || p->pixelRatio() != pixel_ratio_;
	}

	void resetScreen()
	{
		delete screen_;
		pixel_ratio_ = p->pixelRatio();
		if (lyxrc.use_qimage) {
			QImage *x = new QImage(pixel_ratio_ * p->viewport()->width(),
				pixel_ratio_ * p->viewport()->height(), QImage::Format_ARGB32_Premultiplied);
#if QT_VERSION > 0x050000
			x->setDevicePixelRatio(pixel_ratio_);
#endif
			screen_ = x;
		} else {
			QPixmap *x = new QPixmap(pixel_ratio_ * p->viewport()->width(),
				pixel_ratio_ * p->viewport()->height());
#if QT_VERSION > 0x050000
			x->setDevicePixelRatio(pixel_ratio_);
#endif
			screen_ = x;
		}
	}
	///
	GuiWorkArea * p;
	///
	QPaintDevice * screen_;
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
	/// Ratio between physical pixels and device-independent pixels
	/// We save the last used value to detect changes of the
	/// current pixel_ratio of the viewport.
	double pixel_ratio_;
	///
	GuiCompleter * completer_;

	/// Special mode in which Esc and Enter (with or without Shift)
	/// are ignored
	bool dialog_mode_;
	/// store the name of the context menu when the mouse is
	/// pressed. This is used to get the correct context menu 
	/// when the menu is actually shown (after releasing on Windows)
	/// and after the DEPM has done its job.
	std::string context_menu_name_;
}; // GuiWorkArea

} // namespace frontend
} // namespace lyx

#endif // WORKAREA_H
