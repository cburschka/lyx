/*
 * emptytable.C
 * (C) 2000 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "emptytable.h"

/**
 * A simple widget for a quick "preview" in TabularCreateDialog
 */

const unsigned int cellsize = 20;

EmptyTable::EmptyTable(QWidget * parent, const char * name)
	: QTableView(parent,name)
{
	setBackgroundMode(NoBackground);
	setNumCols(5);
	setNumRows(5);
	setCellWidth(cellsize);
	setCellHeight(cellsize);
	setTableFlags(Tbl_autoScrollBars);
}

void EmptyTable::paintCell(QPainter *p, int row, int col)
{
	int x2 = cellWidth(col) - 1;
	int y2 = cellHeight(row) - 1;

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
	setAutoUpdate(true);
	update();
	emit rowsChanged(nr_rows);
}

void EmptyTable::mouseMoveEvent(QMouseEvent *ev)
{
	int x = ev->pos().x();
	int y = ev->pos().y();

	if (x > 0)
		setNumberColumns(x / cellsize + leftCell());

	if (y > 0)
		setNumberRows(y / cellsize + topCell());
}
