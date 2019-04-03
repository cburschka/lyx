// -*- C++ -*-
/**
 * \file GuiSetBorder.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 * \author Jürgen Spitzmüller
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
	GuiSetBorder(QWidget * parent = nullptr, Qt::WindowFlags fl = nullptr);

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
	BorderState getTopLeftTrim();
	BorderState getTopRightTrim();
	BorderState getBottomLeftTrim();
	BorderState getBottomRightTrim();

	bool leftLineSet() { return getLeft() ==  LINE_SET; }
	bool rightLineSet() { return getRight() ==  LINE_SET; }
	bool topLineSet() { return getTop() ==  LINE_SET; }
	bool bottomLineSet() { return getBottom() ==  LINE_SET; }

	bool leftLineUnset() { return getLeft() ==  LINE_UNSET; }
	bool rightLineUnset() { return getRight() ==  LINE_UNSET; }
	bool topLineUnset() { return getTop() ==  LINE_UNSET; }
	bool bottomLineUnset() { return getBottom() ==  LINE_UNSET; }

	bool topLineLTSet() { return getTopLeftTrim() ==  LINE_SET; }
	bool bottomLineLTSet() { return getBottomLeftTrim() ==  LINE_SET; }
	bool topLineRTSet() { return getTopRightTrim() ==  LINE_SET; }
	bool bottomLineRTSet() { return getBottomRightTrim() ==  LINE_SET; }

	bool topLineLTUnset() { return getTopLeftTrim() ==  LINE_UNSET; }
	bool bottomLineLTUnset() { return getBottomLeftTrim() ==  LINE_UNSET; }
	bool topLineRTUnset() { return getTopRightTrim() ==  LINE_UNSET; }
	bool bottomLineRTUnset() { return getBottomRightTrim() ==  LINE_UNSET; }

Q_SIGNALS:
	void rightSet();
	void leftSet();
	void topSet();
	void bottomSet();
	void topLTSet();
	void bottomLTSet();
	void topRTSet();
	void bottomRTSet();
	void clicked();

public Q_SLOTS:
	void setLeftEnabled(bool);
	void setRightEnabled(bool);
	void setTopEnabled(bool);
	void setBottomEnabled(bool);
	void setTopLeftTrimEnabled(bool);
	void setTopRightTrimEnabled(bool);
	void setBottomLeftTrimEnabled(bool);
	void setBottomRightTrimEnabled(bool);
	void setLeft(BorderState);
	void setRight(BorderState);
	void setTop(BorderState);
	void setBottom(BorderState);
	void setTopLeftTrim(BorderState);
	void setTopRightTrim(BorderState);
	void setBottomLeftTrim(BorderState);
	void setBottomRightTrim(BorderState);
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
	void undrawWideTopLine();
	void drawBottom(BorderState);
	void undrawWideBottomLine();
	void drawTopLeftTrim(BorderState);
	void drawTopRightTrim(BorderState);
	void drawBottomLeftTrim(BorderState);
	void drawBottomRightTrim(BorderState);

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
	/// trim areas
	Border top_trim_left_;
	Border top_trim_right_;
	Border bottom_trim_left_;
	Border bottom_trim_right_;

	int margin;
	int corner_length;
	int bwidth;
	int bheight;

	QPixmap buffer;

	bool bottom_drawn_wide_;
	bool top_drawn_wide_;
};


//} // namespace lyx

#endif // GUISETBORDER_H
