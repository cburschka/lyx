// -*- C++ -*-
/**
 * \file GuiWorkArea.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef WORKAREA_H
#define WORKAREA_H

#include "frontends/WorkArea.h"

#include "funcrequest.h"
#include "frontends/Timeout.h"

#include <QAbstractScrollArea>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QTimer>

#include <queue>

class QWidget;
class QDragEnterEvent;
class QDropEvent;
class QWheelEvent;
class QPaintEvent;

namespace lyx {
namespace frontend {

class GuiView;
class QLPainter;

/// for emulating triple click
class double_click {
public:
	int x;
	int y;
	Qt::MouseButton state;
	bool active;

	bool operator==(QMouseEvent const & e) {
		return x == e.x() && y == e.y()
			&& state == e.button();
	}

	double_click()
		: x(0), y(0), state(Qt::NoButton), active(false) {}

	double_click(QMouseEvent * e)
		: x(e->x()), y(e->y()),
		state(e->button()), active(true) {}
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
	int x_old;
	int y_old;
	double scrollbar_value_old;
};

/**
 * Qt-specific implementation of the work area
 * (buffer view GUI)
*/
	class CursorWidget;
class GuiWorkArea : public QAbstractScrollArea, public WorkArea
{
	Q_OBJECT

public:
	///
	GuiWorkArea(int width, int height, int id, LyXView & lyx_view);

	/// return the width of the content pane
	virtual int width() const { return viewport()->width(); }
	/// return the height of the content pane
	virtual int height() const { return viewport()->height(); }
	///
	virtual void setScrollbarParams(int height, int pos, int line_height);

	/// update the passed area.
	void update(int x, int y, int w, int h);

	/// copies specified area of pixmap to screen
	virtual void expose(int x, int y, int exp_width, int exp_height);

	/// paint the cursor and store the background
	virtual void showCursor(int x, int y, int h, CursorShape shape);

	/// hide the cursor
	virtual void removeCursor();

private:
	void doGreyOut(QLPainter & pain);
	///
	void dragEnterEvent(QDragEnterEvent * ev);
	///
	void dropEvent(QDropEvent * ev);
	/// repaint part of the widget
	void paintEvent(QPaintEvent * ev);
	/// widget has been resized
	void resizeEvent(QResizeEvent * ev);
	/// mouse button press
	void mousePressEvent(QMouseEvent * ev);
	/// mouse button release
	void mouseReleaseEvent(QMouseEvent * ev);
	/// mouse double click of button
	void mouseDoubleClickEvent(QMouseEvent * ev);
	/// mouse motion
	void mouseMoveEvent(QMouseEvent * ev);
	/// wheel event
	void wheelEvent(QWheelEvent * ev);
	/// key press
	void keyPressEvent(QKeyEvent * ev);
	/// IM events
	void inputMethodEvent(QInputMethodEvent * ev);

public Q_SLOTS:
	/// Timeout event Slot for keyboard bufferring.
	/// \todo This is not used currently in the code, remove?
	void keyeventTimeout();

	/// Adjust the LyX buffer view with the position of the scrollbar.
	/**
	* The action argument is not used in the the code, it is there
	* only for the connection to the vertical srollbar signal which
	* emits an 'int' action.
	*/
	void adjustViewWithScrollBar(int action = 0);

private:
	/// The slot connected to SyntheticMouseEvent::timeout.
	void generateSyntheticMouseEvent();

	///
	SyntheticMouseEvent synthetic_mouse_event_;
	/// \todo remove
	QTimer step_timer_;
	/// \todo remove
	std::queue<boost::shared_ptr<QKeyEvent> > keyeventQueue_;
	///
	double_click dc_event_;

	///
	bool show_hcursor_;
	///
	bool show_vcursor_;
	///
	bool lshape_cursor_;
	///
	QColor cursor_color_;
	///
	CursorShape cursor_shape_;
	///	
	CursorWidget * cursor_;
};

} // namespace frontend
} // namespace lyx

#endif // WORKAREA_H
