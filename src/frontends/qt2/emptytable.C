/**
 * \file emptytable.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include "emptytable.h"

/**
 * A simple widget for a quick "preview" in TabularCreateDialog
 */

const unsigned int cellsize = 20;

EmptyTable::EmptyTable(QWidget * parent, const char * name)
	: QtTableView(parent,name)
{
	setNumCols(5);
	setNumRows(5);
	setCellWidth(cellsize);
	setCellHeight(cellsize);
	setTableFlags(Tbl_autoScrollBars);
}

QSize EmptyTable::sizeHint() const
{
	return QSize(cellsize * numCols(), cellsize * numRows());
}


void EmptyTable::paintCell(QPainter *p, int row, int col)
{
	int const x2 = cellWidth(col) - 1;
	int const y2 = cellHeight(row) - 1;

	p->fillRect(0, 0, x2, y2, QColor("white"));
	p->drawLine(x2, 0, x2, y2);
	p->drawLine(0, y2, x2, y2);
}

void EmptyTable::setNumberColumns(int nr_cols)
{
	if (nr_cols < 1)
		return;
	if (nr_cols == numCols())
		return;
	setAutoUpdate(false);
	setNumCols(nr_cols);
	updateGeometry();
	setAutoUpdate(true);
	update();
	emit colsChanged(nr_cols);
}

void EmptyTable::setNumberRows(int nr_rows)
{
	if (nr_rows < 1)
		return;
	if (nr_rows == numRows())
		return;
	setAutoUpdate(false);
	setNumRows(nr_rows);
	updateGeometry();
	setAutoUpdate(true);
	update();
	emit rowsChanged(nr_rows);
}

void EmptyTable::mouseMoveEvent(QMouseEvent *ev)
{
	int const x = ev->pos().x();
	int const y = ev->pos().y();

	if (x > 0)
		setNumberColumns(x / cellsize + leftCell());

	if (y > 0)
		setNumberRows(y / cellsize + topCell());
}
