/**
 * \file EmptyTable.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "EmptyTable.h"

#include <QPainter>
#include <QMouseEvent>

/**
 * A simple widget for a quick "preview" in TabularCreateDialog
 */

unsigned int const cellsize = 20;


EmptyTable::EmptyTable(QWidget * parent, int rows, int columns)
	: QTableWidget(rows, columns, parent)
{
	resetCellSize();
	setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	viewport()->resize(cellsize*rows,cellsize*columns);
}


QSize EmptyTable::sizeHint() const
{
	return QSize(cellsize * (2+columnCount()), cellsize * (2+rowCount()));
}

void EmptyTable::resetCellSize()
{
	for(int i=0; i<rowCount(); ++i)
		setRowHeight(i, cellsize);
	for(int i=0; i<columnCount(); ++i)
		setColumnWidth(i, cellsize);
}

void EmptyTable::paintCell(QPainter * p, int row, int col)
{
	int const x2 = columnWidth(col) - 1;
	int const y2 = rowHeight(row) - 1;

	p->fillRect(0, 0, x2, y2, QColor("white"));
	p->drawLine(x2, 0, x2, y2);
	p->drawLine(0, y2, x2, y2);

	if (row + 1 != rowCount() || col + 1 != columnCount())
		return;

	// draw handle
	int const step = cellsize / 5;
	int const space = 4;
	int x = cellsize - step;
	int const y = cellsize - space;
	int const ex = cellsize - space;
	int ey = cellsize - step;
	while (x > space) {
		p->drawLine(x, y, ex, ey);
		x -= step;
		ey -= step;
	}
}


void EmptyTable::setNumberColumns(int nr_cols)
{
	if (nr_cols < 1)
		return;
	if (nr_cols == columnCount())
		return;
	setColumnCount(nr_cols);
	resetCellSize();
	updateGeometry();
	// emit signal
	colsChanged(nr_cols);
}


void EmptyTable::setNumberRows(int nr_rows)
{
	if (nr_rows < 1)
		return;
	if (nr_rows == rowCount())
		return;
	setRowCount(nr_rows);
	resetCellSize();
	updateGeometry();
	// emit signal
	rowsChanged(nr_rows);
}

/*
void EmptyTable::mouseMoveEvent(QMouseEvent *ev)
{
	int const x = ev->pos().x();
	int const y = ev->pos().y();

	if (x > 0)
		setNumberColumns(x / cellsize + columnCount()-1);

	if (y > 0)
		setNumberRows(y / cellsize + rowCount()-1);
}
*/

#include "moc_EmptyTable.cpp"


namespace lyx {


} // namespace lyx
