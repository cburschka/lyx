/**
 * \file emptytable.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef EMPTYTABLE_H
#define EMPTYTABLE_H

#include <qpainter.h> 
#include <qtableview.h> 

/**
 * A simple widget for a quick "preview" in TabularCreateDialog.
 * The user can drag on the widget to change the table dimensions.
 */
class EmptyTable : public QTableView {
	Q_OBJECT
public:
	EmptyTable(QWidget * parent = 0, const char * name = 0);
	
	~EmptyTable() {};

	virtual QSize sizeHint() const;
 
public slots:
	/// set the number of columns in the table and emit colsChanged() signal 
	void setNumberColumns(int nr_cols);
	/// set the number of rows in the table and emit rowsChanged() signal 
	void setNumberRows(int nr_rows);
 
signals:
	/// the number of columns changed
	void colsChanged(int);
	/// the number of rows changed
	void rowsChanged(int);

protected:
	/// fill in a cell
	virtual void paintCell(class QPainter *, int, int);
	virtual void mouseMoveEvent(QMouseEvent *);
	
private:
	/// number of current columns 
	unsigned int cols;
	/// number of current rows
	unsigned int rows;
};

#endif
