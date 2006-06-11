// -*- C++ -*-
/**
 * \file InsertTableWidget.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */


#ifndef INSERTTABLEWIDGET_H
#define INSERTTABLEWIDGET_H

#include "frontends/LyXView.h"
#include <QWidget>

namespace lyx {
namespace frontend {


class InsertTableWidget : public QWidget {
	Q_OBJECT
public:

	InsertTableWidget(LyXView &, QWidget *);

signals:
	//! widget is visible
	void visible(bool);

public slots:
	//! show the widget
	void show(bool);
	//! enable/disable parent
	void updateParent();

protected slots:
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mousePressEvent(QMouseEvent *);
	void paintEvent(QPaintEvent *);
	
private:
	//! update the geometry
	void resetGeometry();
	//! initialize parameters to default values
	void init();
	//! draw the grid
	void drawGrid(int rows, int cols, Qt::GlobalColor color);

	//! colwidth in pixels
	int colwidth_;
	//! rowheight in pixels
	int rowheight_;
	//! total rows
	int rows_;
	//! total cols
	int cols_;
	//! row of pointer
	int bottom_;
	//! column of pointer
	int right_;	
	//! the lyxview we need to dispatch the funcrequest
	LyXView & lyxView_;
	//! widget under mouse
	bool underMouse_;
};

} // namespace frontend
} // namespace lyx

#endif // INSERTTABLEWIDGET_H
