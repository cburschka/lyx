// -*- C++ -*-
/**
 * \file EmptyTable.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef EMPTYTABLE_H
#define EMPTYTABLE_H

#include <QTableWidget>


class QMouseEvent;

//namespace lyx {

/**
 * A simple widget for a quick "preview" in TabularCreateDialog.
 * The user can drag on the widget to change the table dimensions.
 */
class EmptyTable : public QTableWidget {
	Q_OBJECT
public:
	EmptyTable(QWidget * parent = 0, int rows = 5, int columns = 5);

	virtual QSize sizeHint() const;
public Q_SLOTS:
	/// set the number of columns in the table and emit colsChanged() signal
	void setNumberColumns(int nr_cols);
	/// set the number of rows in the table and emit rowsChanged() signal
	void setNumberRows(int nr_rows);
Q_SIGNALS:
	/// the number of columns changed
	void colsChanged(int);
	/// the number of rows changed
	void rowsChanged(int);
protected:
	/// fill in a cell
	virtual void paintCell(class QPainter *, int, int);
	virtual void mouseMoveEvent(QMouseEvent *);

	/// Reset all the cell size to default
	virtual void resetCellSize();

private:
};


//} // namespace lyx

#endif
