// -*- C++ -*-
/**
 * \file QBrowseBox.h
 *
 * Original file taken from klyx 0.10 sources:
 * $Id: QBrowseBox.h,v 1.4 2002/10/20 01:48:27 larsbj Exp $
 *
 * \author Kalle Dalheimer ?
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QBROWSEBOX_H
#define QBROWSEBOX_H

#include "qgridview.h"

#include <qwidget.h>


class QString;
class QPainter;
class QPixmap;


class QBrowseBox : public QGridView
{
	Q_OBJECT
public:
	QBrowseBox(int rows, int cols);
	~QBrowseBox();
	
	void insertItem(QString const & text, int x, int y);
	void insertItem(char const * text, int x, int y);
	void insertItem(QPixmap pixmap, int x, int y);
	void insertItem(QPixmap pixmap);
	void removeItem(int x, int y);
	void clear();
	
	QString text(int x, int y);
	QPixmap pixmap(int x, int y);
	
	//  int exec(QPoint const & pos);
	//  int exec(int x, int y);
	//  int exec(QWidget * trigger);
	
signals:
        void selected(int, int);
	
protected:
	virtual void keyPressEvent(QKeyEvent * e);
	virtual void resizeEvent(QResizeEvent * e);
	virtual void mouseReleaseEvent(QMouseEvent * e);
	//  virtual void closeEvent(QCloseEvent * e);
	virtual void mouseMoveEvent(QMouseEvent * e);
	virtual void paintCell(QPainter *, int x, int y);
	
private:
	// make sure the automatically generated one is not used
	QBrowseBox & operator=(QBrowseBox const &);
	
	void moveLeft();
	void moveRight();
	void moveUp();
	void moveDown();
	
	int coordsToIndex(int x, int y);
	
	QString * texts_;
	QPixmap* pixmaps_;
	QPoint activecell_;
};
#endif
