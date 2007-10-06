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

#include "FuncRequest.h"
#include "support/Timeout.h"

#include <QAbstractScrollArea>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QTabWidget>
#include <QTimer>

#include <queue>

class QWidget;
class QDragEnterEvent;
class QDropEvent;
class QWheelEvent;
class QPaintEvent;

namespace lyx {
namespace frontend {

/// for emulating triple click
class double_click {
public:
	Qt::MouseButton state;
	bool active;

	bool operator==(QMouseEvent const & e) {
		return state == e.button();
	}

	double_click()
		: state(Qt::NoButton), active(false) {}

	double_click(QMouseEvent * e)
		: state(e->button()), active(true) {}
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
	GuiWorkArea(Buffer & buffer, LyXView & lv);

	///
	bool hasFocus() const { return QAbstractScrollArea::hasFocus(); }
	bool isVisible() const { return QAbstractScrollArea::isVisible(); }

	/// return the width of the content pane
	virtual int width() const { return viewport()->width(); }
	/// return the height of the content pane
	virtual int height() const { return viewport()->height(); }
	///
	virtual void setScrollbarParams(int height, int pos, int line_height);
	///
	virtual void scheduleRedraw() { schedule_redraw_ = true; }

	/// update the passed area.
	void update(int x, int y, int w, int h);

	/// copies specified area of pixmap to screen
	virtual void expose(int x, int y, int exp_width, int exp_height);

	/// paint the cursor and store the background
	virtual void showCursor(int x, int y, int h, CursorShape shape);

	/// hide the cursor
	virtual void removeCursor();

private:
	///
	void focusInEvent(QFocusEvent *);
	///
	void focusOutEvent(QFocusEvent *);
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
	/// IM query
	QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

public Q_SLOTS:
	/// Adjust the LyX buffer view with the position of the scrollbar.
	/**
	* The action argument is not used in the the code, it is there
	* only for the connection to the vertical srollbar signal which
	* emits an 'int' action.
	*/
	void adjustViewWithScrollBar(int action = 0);
	/// timer to limit triple clicks
	void doubleClickTimeout();

private:
	/// The slot connected to SyntheticMouseEvent::timeout.
	void generateSyntheticMouseEvent();

	///
	SyntheticMouseEvent synthetic_mouse_event_;
	///
	double_click dc_event_;

	///
	CursorWidget * cursor_;
	///
	void updateScreen();
	///
	QPixmap screen_;
	///
	bool need_resize_;
	///
	bool schedule_redraw_;
	///
	int preedit_lines_;
}; //GuiWorkArea

/// A tabbed set of GuiWorkAreas.
class TabWorkArea : public QTabWidget
{
	Q_OBJECT
public:
	TabWorkArea(QWidget * parent = 0);
	void showBar(bool show);

Q_SIGNALS:
	///
	void currentWorkAreaChanged(GuiWorkArea *);

public Q_SLOTS:
	///
	void on_currentTabChanged(int index);
	///
	void closeCurrentTab();
}; // TabWorkArea

} // namespace frontend
} // namespace lyx

#endif // WORKAREA_H
