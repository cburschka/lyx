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

#include "support/debug.h"

#include <QPainter>
#include <QMouseEvent>

/**
 * A simple widget for a quick "preview" in TabularCreateDialog
 */

unsigned int const cellheight = 20;
unsigned int const cellwidth = 30;


EmptyTable::EmptyTable(QWidget * parent, int rows, int columns)
	: QTableWidget(rows, columns, parent)
{
	resetCellSize();
	setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	viewport()->resize(cellheight * rows, cellwidth * columns);
	setSelectionMode(QAbstractItemView::NoSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
}


QSize EmptyTable::sizeHint() const
{
	return QSize(cellwidth * (2 + columnCount()), cellheight * (2 + rowCount()));
}

void EmptyTable::resetCellSize()
{
	for(int i = 0; i < rowCount(); ++i)
		setRowHeight(i, cellheight);
	for(int i = 0; i < columnCount(); ++i)
		setColumnWidth(i, cellwidth);
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
	int const step = cellheight / 5;
	int const space = 4;
	int x = cellwidth - step;
	int const y = cellheight - space;
	int const ex = cellwidth - space;
	int ey = cellheight - step;
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


void EmptyTable::mouseMoveEvent(QMouseEvent *ev)
{
	int cc = columnCount();
	int rc = rowCount();
	int x = ev->x();
	int y = ev->y();
	int w = cellwidth * cc;
	int h = cellheight * rc;
	int wl = cellwidth * (cc - 1);
	int hl = cellheight * (rc - 1);
	if (x > w)
		setNumberColumns(cc + 1);
	if (y > h)
		setNumberRows(rc + 1);
	if (x < wl)
		setNumberColumns(cc - 1);
	if (y < hl)
		setNumberRows(rc - 1);
}

#include "moc_EmptyTable.cpp"


namespace lyx {


} // namespace lyx
