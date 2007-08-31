// -*- C++ -*-
/**
 * \file GuiSetBorder.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */
#ifndef QSETBORDER_H
#define QSETBORDER_H

#include <QWidget>
#include <QPixmap>

class QColor;
class QMouseEvent;
class QPaintEvent;

//namespace lyx {

class GuiSetBorder : public QWidget
{
	Q_OBJECT
public:
	GuiSetBorder(QWidget * parent = 0, Qt::WFlags fl = 0);

	bool getLeft();
	bool getRight();
	bool getTop();
	bool getBottom();

Q_SIGNALS:
	void rightSet(bool);
	void leftSet(bool);
	void topSet(bool);
	void bottomSet(bool);
	void clicked();

public Q_SLOTS:
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

	class Border {
	public:
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


//} // namespace lyx

#endif // QSETBORDER_H
