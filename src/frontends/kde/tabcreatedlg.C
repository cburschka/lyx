/**
 * \file tabcreatedlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include "tabcreatedlg.h"
#include "support/lstrings.h"

#include <gettext.h>
#include <qtooltip.h>

TabularCreateDialog::TabularCreateDialog (FormTabularCreate * form, QWidget * parent, char const * name)
	: TabularCreateDialogData(parent, name), form_(form)
{
	setCaption(name);
	table->setMinimumSize(100,100); 
	rows->setValue(5);
	cols->setValue(5);
	connect(rows,SIGNAL(valueChanged(int)),table,SLOT(setNumberRows(int)));
	connect(cols,SIGNAL(valueChanged(int)),table,SLOT(setNumberColumns(int)));
	connect(table,SIGNAL(colsChanged(unsigned int)),this,SLOT(colsChanged(unsigned int))); 
	connect(table,SIGNAL(rowsChanged(unsigned int)),this,SLOT(rowsChanged(unsigned int))); 
	QToolTip::add(table, _("Drag with left mouse button to resize")); 
}


TabularCreateDialog::~TabularCreateDialog()
{
}


void TabularCreateDialog::colsChanged(unsigned int nr_cols)
{
	if (nr_cols != strToUnsignedInt(cols->text()))
		cols->setValue(nr_cols);
}


void TabularCreateDialog::rowsChanged(unsigned int nr_rows)
{
	if (nr_rows != strToUnsignedInt(rows->text()))
		rows->setValue(nr_rows);
}


void TabularCreateDialog::clickedInsert()
{
	form_->apply(strToInt(rows->text()), strToInt(cols->text()));
	form_->close();
	hide();
}

 
void TabularCreateDialog::clickedCancel()
{
	form_->close();
	hide();
}
