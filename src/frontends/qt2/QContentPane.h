// -*- C++ -*-
/**
 * \file QContentPane.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QCONTENTPANE_H
#define QCONTENTPANE_H

#ifdef __GNUG__
#pragma interface
#endif

#include <qwidget.h>
#include <qevent.h>
#include <qpixmap.h>

#include <boost/scoped_ptr.hpp>

class QWorkArea;

/// for emulating triple click
struct double_click {
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


/**
 * Widget for actually drawing the document on
 */
class QContentPane : public QWidget {
	Q_OBJECT
public:
	QContentPane(QWorkArea * parent);

	/// return the backing pixmap
	QPixmap * pixmap() const { return pixmap_.get(); }
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

	/// key press
	void keyPressEvent(QKeyEvent * e);
public slots:
	void doubleClickTimeout();

	void scrollBarChanged(int);
private:
	/// owning widget
	QWorkArea * wa_;

	/// the double buffered pixmap
	boost::scoped_ptr<QPixmap> pixmap_;

	double_click dc_event_;
};

#endif // QCONTENTPANE_H
