// -*- C++ -*-
/**
 * \file QBrowseBox.h
 *
 * Original file taken from klyx 0.10 sources:
 *
 * \author Kalle Dalheimer
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
	QBrowseBox(int rows, int cols, QWidget* parent=0, char const * name=0, WFlags f=0);
	~QBrowseBox();
	
	void insertItem(QString const & text, int row, int col);
	void insertItem(char const * text, int row, int col);
	void insertItem(QPixmap pixmap, int row, int col);
	void insertItem(QPixmap pixmap);
	void removeItem(int row, int col);
	void clear();
	
	QString text(int row, int col);
	QPixmap pixmap(int row, int col);
	
	int exec(QPoint const & pos);
	int exec(int x, int y);
	int exec(QWidget const * trigger);
	
signals:
        void selected(int, int);
	
protected:
	virtual void keyPressEvent(QKeyEvent * e);
	virtual void resizeEvent(QResizeEvent *e);
	virtual void contentsMouseReleaseEvent(QMouseEvent *);
	virtual void closeEvent(QCloseEvent * e);
	virtual void contentsMouseMoveEvent(QMouseEvent * e);
	virtual void paintCell(QPainter *, int row, int col);
	
private:
	// make sure the automatically generated one is not used
	QBrowseBox & operator=(QBrowseBox const &);

	void moveLeft();
	void moveRight();
	void moveUp();
	void moveDown();
	
	int coordsToIndex(int row, int col);
	
	QString * texts_;
	QPixmap* pixmaps_;
	QPoint activecell_;
	bool firstrelease_;
	bool inloop;
	
};
#endif
