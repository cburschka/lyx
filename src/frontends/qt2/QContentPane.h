// -*- C++ -*-
/**
 * \file QContentPane.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QCONTENTPANE_H
#define QCONTENTPANE_H

#ifdef emit
#undef emit
#endif

#include "funcrequest.h"
#include "frontends/Timeout.h"

#include <qwidget.h>
#include <qpixmap.h>
#include <qtimer.h>

#include <boost/scoped_ptr.hpp>

#include <queue>

#if (defined(Q_WS_X11) && QT_VERSION >= 0x030200)
#define USE_INPUT_METHODS 1
#endif

class QWorkArea;

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
 * Widget for actually drawing the document on
 */
class QContentPane : public QWidget {
	Q_OBJECT
public:
	QContentPane(QWorkArea * parent);

	/// return the backing pixmap
	QPixmap * pixmap() const { return pixmap_.get(); }
	/// track scrollbar signals?
	void trackScrollbar(bool track_on);

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
	/// IM events
	void imStartEvent(QIMEvent *);
	void imComposeEvent(QIMEvent *);
	void imEndEvent(QIMEvent *);
#endif
public slots:
	void doubleClickTimeout();

	void scrollBarChanged(int);
	void keyeventTimeout();

private:
	/// The slot connected to SyntheticMouseEvent::timeout.
	void generateSyntheticMouseEvent();
	SyntheticMouseEvent synthetic_mouse_event_;

	///
	bool track_scrollbar_;
	/// owning widget
	QWorkArea * wa_;

	QTimer step_timer_;
	std::queue<boost::shared_ptr<QKeyEvent> > keyeventQueue_;

	/// the double buffered pixmap
	boost::scoped_ptr<QPixmap> pixmap_;

	double_click dc_event_;
};

#endif // QCONTENTPANE_H
