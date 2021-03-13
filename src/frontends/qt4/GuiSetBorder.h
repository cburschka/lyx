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

#ifndef GUISETBORDER_H
#define GUISETBORDER_H

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
	GuiSetBorder(QWidget * parent = 0, Qt::WindowFlags fl = {});

	// We need tristate for multi-cell selection
	enum BorderState {
		LINE_UNSET,
		LINE_SET,
		LINE_UNDECIDED,
		LINE_UNDEF
	};

	BorderState getLeft();
	BorderState getRight();
	BorderState getTop();
	BorderState getBottom();

	bool leftLineSet() { return getLeft() ==  LINE_SET; }
	bool rightLineSet() { return getRight() ==  LINE_SET; }
	bool topLineSet() { return getTop() ==  LINE_SET; }
	bool bottomLineSet() { return getBottom() ==  LINE_SET; }

	bool leftLineUnset() { return getLeft() ==  LINE_UNSET; }
	bool rightLineUnset() { return getRight() ==  LINE_UNSET; }
	bool topLineUnset() { return getTop() ==  LINE_UNSET; }
	bool bottomLineUnset() { return getBottom() ==  LINE_UNSET; }

Q_SIGNALS:
	void rightSet();
	void leftSet();
	void topSet();
	void bottomSet();
	void clicked();

public Q_SLOTS:
	void setLeftEnabled(bool);
	void setRightEnabled(bool);
	void setTopEnabled(bool);
	void setBottomEnabled(bool);
	void setLeft(BorderState);
	void setRight(BorderState);
	void setTop(BorderState);
	void setBottom(BorderState);
	void setAll(BorderState);

protected:
	void mousePressEvent(QMouseEvent * e);
	void paintEvent(QPaintEvent * e);

private:
	void init();

	void drawLine(QColor const & col, int x, int y, int x2, int y2);

	void drawLeft(BorderState);
	void drawRight(BorderState);
	void drawTop(BorderState);
	void drawBottom(BorderState);

	class Border {
	public:
		Border() : set(LINE_SET), enabled(true) {}
		BorderState set;
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

#endif // GUISETBORDER_H
