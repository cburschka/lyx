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
	spin_rows->setValue(5);
	spin_cols->setValue(5);
	connect(spin_rows,SIGNAL(valueChanged(int)),table,SLOT(setNumberRows(int)));
	connect(spin_cols,SIGNAL(valueChanged(int)),table,SLOT(setNumberColumns(int)));
	connect(table,SIGNAL(colsChanged(unsigned int)),this,SLOT(colsChanged(unsigned int))); 
	connect(table,SIGNAL(rowsChanged(unsigned int)),this,SLOT(rowsChanged(unsigned int))); 
	QToolTip::add(table, _("Drag with left mouse button to resize")); 
}


TabularCreateDialog::~TabularCreateDialog()
{
}


void TabularCreateDialog::colsChanged(unsigned int nr_cols)
{
	if (nr_cols != strToUnsignedInt(spin_cols->text()))
		spin_cols->setValue(nr_cols);
}


void TabularCreateDialog::rowsChanged(unsigned int nr_rows)
{
	if (nr_rows != strToUnsignedInt(spin_rows->text()))
		spin_rows->setValue(nr_rows);
}


void TabularCreateDialog::clickedInsert()
{
	form_->OKButton();
}

 
void TabularCreateDialog::clickedCancel()
{
	form_->CancelButton(); 
}


void TabularCreateDialog::closeEvent(QCloseEvent * e)
{
	form_->CancelButton();
	e->accept();
}
