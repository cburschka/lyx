// -*- C++ -*-
/**
 * \file QSetBorder.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */
#ifndef QSETBORDER_H
#define QSETBORDER_H

#include <qwidget.h>
#include <qpixmap.h>

class QMouseEvent;
class QResizeEvent;
class QPaintEvent;

class QSetBorder : public QWidget
{
	Q_OBJECT
public:
	QSetBorder(QWidget * parent = 0, char const * name = 0, WFlags fl = 0);

	bool getLeft();
	bool getRight();
	bool getTop();
	bool getBottom();

signals:
	void rightSet(bool);
	void leftSet(bool);
	void topSet(bool);
	void bottomSet(bool);

public slots:
	void setLeft(bool);
	void setRight(bool);
	void setTop(bool);
	void setBottom(bool);

protected:
	void mousePressEvent(QMouseEvent * e);
	void paintEvent(QPaintEvent * e);
	
private:
	void init();

	void drawLeft(bool);
	void drawRight(bool);
	void drawTop(bool);
	void drawBottom(bool);

	bool left_;
	bool right_;
	bool top_;
	bool bottom_;

	int m;
	int l;
	int w;
	int h;
	
	QPixmap buffer;
};
 
#endif // QSETBORDER_H
