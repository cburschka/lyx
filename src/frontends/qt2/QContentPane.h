// -*- C++ -*-
/**
 * \file QContentPane.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QCONTENTPANE_H
#define QCONTENTPANE_H

#ifdef __GNUG__
#pragma interface
#endif

#include <config.h>
#include <utility>
#include <boost/smart_ptr.hpp>

#include <qwidget.h>
#include <qscrollbar.h>
#include <qpixmap.h>
#include <qevent.h>

class QWorkArea; 
 
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
	/// keyboard focus in this widget
	void focusInEvent(QFocusEvent * e);
	/// keyboard focus lost
	void focusOutEvent(QFocusEvent * e);
 
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
	void scrollBarChanged(int);
 
private:
	/// owning widget
	QWorkArea * wa_;
 
	/// the double buffered pixmap
	boost::scoped_ptr<QPixmap> pixmap_;
};

#endif // QCONTENTPANE_H
