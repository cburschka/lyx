// -*- C++ -*-
/**
 * \file QBrowseBox.h
 *
 * Original file taken from klyx 0.10 sources:
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBROWSEBOX_H
#define QBROWSEBOX_H

#include "qgridview.h"


class QString;
class QPainter;
class QPixmap;


class QBrowseBox : public QGridView
{
	Q_OBJECT
public:
	QBrowseBox(int rows, int cols, QWidget * parent = 0, const char * name = 0, WFlags f = 0);
	~QBrowseBox();

	void insertItem(QPixmap pixmap);

	QPixmap pixmap(int row, int col);

	int exec(const QPoint & pos);
	int exec(int x, int y);
	int exec(const QWidget * trigger);

signals:
	void selected(int, int);

protected:
	virtual void keyPressEvent(QKeyEvent * e);
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

	void insertItem(QPixmap pixmap, int row, int col);

	int coordsToIndex(int row, int col);

	QPixmap* pixmaps_;
	QPoint activecell_;
	bool firstrelease_;
	bool inloop;

};
#endif
