// -*- C++ -*-
/**
 * \file QWorkArea.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QWORKAREA_H
#define QWORKAREA_H

#ifdef __GNUG__
#pragma interface
#endif

#include <utility>

#include "WorkArea.h"
#include "QLPainter.h"
#include "QContentPane.h"
 
#include <qwidget.h>
#include <qscrollbar.h>
#include <qpixmap.h>

 
/**
 * Qt-specific implementation of the work area
 * (buffer view GUI)
 *
 * It consists of a content pane widget, and a scrollbar.
 * Hopefully soon we can just use QScrollView ...
 */
class QWorkArea : public WorkArea, public QWidget {
 
	friend class QContentPane;
 
public:

	QWorkArea(int x, int y, int w, int h);

	// FIXME:
	virtual void greyOut() const { }

	virtual ~QWorkArea();
	/// return this widget's painter
	virtual Painter & getPainter() { return painter_; }
	/// return the width of the content pane
	virtual int workWidth() const { return content_->width(); }
	/// return the height of the content pane
	virtual int workHeight() const { return content_->height(); }
	/// resize this widget
	virtual void resize(int xpos, int ypos, int width, int height);
	/// FIXME: makes no sense ?
	virtual void redraw() const { }
	/// set focus to this widget
	virtual void setFocus() const { /* FIXME */ }
	/// does this widget have the focus ?
	virtual bool hasFocus() const { /* FIXME */ return true; }
	/// 
	virtual void setScrollbarParams(int height, int pos, int line_height);
 
	/// a selection exists
	virtual void haveSelection(bool) const;
	///
	virtual string const getClipboard() const;
	///
	virtual void putClipboard(string const &) const;

	/// get the pixmap we paint on to
	QPixmap * getPixmap() const { return content_->pixmap(); }

	/// get the content pane widget
	QWidget * getContent() const { return content_; }

private:
	/// scroll bar
	QScrollBar * scrollbar_;
	/// content
	QContentPane * content_;
 
	/// our painter
	QLPainter painter_;
};
 
#endif // QWORKAREA_H
