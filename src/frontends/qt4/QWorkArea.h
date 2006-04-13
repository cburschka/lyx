// -*- C++ -*-
/**
 * \file QWorkArea.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QWORKAREA_H
#define QWORKAREA_H

#if (defined(Q_WS_X11) && QT_VERSION >= 0x030200)
#define USE_INPUT_METHODS 1
#endif

#ifdef emit
#undef emit
#endif

#include "WorkArea.h"
#include "QLPainter.h"
#include "LyXView.h"

#include "funcrequest.h"
#include "frontends/Timeout.h"

#include <QAbstractScrollArea>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QTimer>
#include <QImage>
#include <QPixmap>

#include <queue>

class Painter;

class QWidget;
class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;

/// for emulating triple click
class double_click {
public:
	int x;
	int y;
	Qt::ButtonState state;
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
class QWorkArea : public QAbstractScrollArea, public WorkArea {

	Q_OBJECT

public:

	QWorkArea(LyXView & owner, int w, int h);

	virtual ~QWorkArea();
	/// return the width of the content pane
	virtual int workWidth() const { return workWidth_; }

	/// return the height of the content pane
	virtual int workHeight() const { return workHeight_; }
	///
	virtual void setScrollbarParams(int height, int pos, int line_height);

	/// a selection exists
	virtual void haveSelection(bool) const;

	///
	virtual std::string const getClipboard() const;

	///
	virtual void putClipboard(std::string const &) const;

	///
	virtual void dragEnterEvent(QDragEnterEvent * event);

	///
	virtual void dropEvent(QDropEvent* event);

	/// return the widget's painter
	virtual Painter & getPainter() { return (Painter &) painter_; }

	/// return the backing pixmap
	QPaintDevice * paintDevice() { return &paint_device_; }

	/// update the passed area.
	void update(int x, int y, int w, int h);

	/// return a screen copy of the defined area.
	QPixmap copyScreen(int x, int y, int w, int h) const;

	/// Draw a pixmap onto the backing pixmap.
	/**
	QPixmap is implicitely shared so no need to pass by reference.
	*/
	void drawScreen(int x, int y, QPixmap pixmap);

        LyXView & view()
        {
                return view_;
        }
protected:

	/// repaint part of the widget
	void paintEvent(QPaintEvent * e);
	/// widget has been resized
	void resizeEvent(QResizeEvent * e);
	/// mouse button press
	void mousePressEvent(QMouseEvent * e);
	/// mouse button release
	void mouseReleaseEvent(QMouseEvent * e);
	/// mouse double click of button
	void mouseDoubleClickEvent(QMouseEvent * e);
	/// mouse motion
	void mouseMoveEvent(QMouseEvent * e);
	/// wheel event
	void wheelEvent(QWheelEvent * e);
	/// key press
	void keyPressEvent(QKeyEvent * e);

#if USE_INPUT_METHODS
protected:
	/// IM events
	void QWorkArea::inputMethodEvent(QInputMethodEvent * e)
#endif

public slots:

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
        ///
        LyXView & view_;
        
	/// Buffer view width.
	int workWidth_;

	/// Buffer view height.
	int workHeight_;

	/// Our painter.
	QLPainter painter_;

	/// The slot connected to SyntheticMouseEvent::timeout.
	void generateSyntheticMouseEvent();

	///
	SyntheticMouseEvent synthetic_mouse_event_;

	/// Our client side painting device.
	QImage paint_device_;

	/// Our server side painting device.
	QPixmap screen_device_;

	/// \todo remove
	QTimer step_timer_;

	/// \todo remove
	std::queue<boost::shared_ptr<QKeyEvent> > keyeventQueue_;

	double_click dc_event_;
};

#endif // QWORKAREA_H
