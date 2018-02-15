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

#include "support/FileName.h"
#include "support/Timeout.h"

#include <QMouseEvent>
#include <QTimer>

namespace lyx {

class Buffer;

namespace frontend {

class GuiCompleter;
class GuiPainter;
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
class CaretWidget;

struct GuiWorkArea::Private
{
	///
	Private(GuiWorkArea *);

	///
	~Private();

	///
	void resizeBufferView();

	///
	void dispatch(FuncRequest const & cmd0);
	/// hide the visible caret, if it is visible
	void hideCaret();
	/// show the caret if it is not visible
	void showCaret();
	/// Set the range and value of the scrollbar and connect to its valueChanged
	/// signal.
	void updateScrollbar();
	/// Change the cursor when the mouse hovers over a clickable inset
	void updateCursorShape();

	void paintPreeditText(GuiPainter & pain);

	///
	GuiWorkArea * p;
	///
	BufferView * buffer_view_;
	///
	GuiView * lyx_view_;

	///
	CaretWidget * caret_;
	/// is the caret currently displayed
	bool caret_visible_;
	///
	QTimer caret_timeout_;

	///
	SyntheticMouseEvent synthetic_mouse_event_;
	///
	DoubleClick dc_event_;

	///
	bool need_resize_;

	/// the current preedit text of the input method
	docstring preedit_string_;
	/// Number of lines used by preedit text
	int preedit_lines_;
	/// the attributes of the preedit text
	QList<QInputMethodEvent::Attribute> preedit_attr_;

	/// Ratio between physical pixels and device-independent pixels
	/// We save the last used value to detect changes of the
	/// current pixel_ratio of the viewport.
	double last_pixel_ratio_;
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

	/// stuff related to window title
	///
	support::FileName file_name_;
	///
	bool shell_escape_;
	///
	bool read_only_;
	///
	docstring vc_status_;
	///
	bool clean_;
	///
	bool externally_modified_;

}; // GuiWorkArea

} // namespace frontend
} // namespace lyx

#endif // WORKAREA_H
