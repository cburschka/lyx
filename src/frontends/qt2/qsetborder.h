// -*- C++ -*-
/**
 * \file qsetborder.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */
#ifndef QSETBORDER_H
#define QSETBORDER_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qcolor.h>

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
	void clicked();

public slots:
	void setLeftEnabled(bool);
	void setRightEnabled(bool);
	void setTopEnabled(bool);
	void setBottomEnabled(bool);
	void setLeft(bool);
	void setRight(bool);
	void setTop(bool);
	void setBottom(bool);
	void setAll(bool);

protected:
	void mousePressEvent(QMouseEvent * e);
	void paintEvent(QPaintEvent * e);

private:
	void init();

	void drawLine(QColor const & col, int x, int y, int x2, int y2);

	void drawLeft(bool);
	void drawRight(bool);
	void drawTop(bool);
	void drawBottom(bool);

	struct Border {
		Border() : set(true), enabled(true) {}
		bool set;
		bool enabled;
	};

	Border left_;
	Border right_;
	Border top_;
	Border bottom_;

	int m;
	int l;
	int w;
	int h;

	QPixmap buffer;
};

#endif // QSETBORDER_H
