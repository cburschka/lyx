/**
 * \file QTabularCreateDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include <config.h>
#include <gettext.h>

#include "QTabularCreateDialog.h"
#include "QTabularCreate.h"
 
#include "support/lstrings.h"

#include <qpushbutton.h>
#include <qspinbox.h>
#include "emptytable.h"
#include <qpainter.h>
#include <qtableview.h>
#include <qtooltip.h>

QTabularCreateDialog::QTabularCreateDialog(QTabularCreate * form, QWidget * parent,  const char * name, bool modal, WFlags fl)
	 : QTabularCreateDialogBase(parent, name, modal, fl), 
	form_(form)
{
	setCaption(name);
	table->setMinimumSize(100,100);
	rows->setValue(5);
	columns->setValue(5);
	QToolTip::add(table, _("Drag with left mouse button to resize"));
}

 
QTabularCreateDialog::~QTabularCreateDialog()
{
	 // no need to delete child widgets, Qt does it all for us
}
 

void QTabularCreateDialog::insert_tabular()
{
	form_->apply((rows->text()).toInt(), (columns->text()).toInt());
	form_->close();
	hide();
}
 

void QTabularCreateDialog::cancel_adaptor()
{
	form_->close();
	hide();
}

 
void QTabularCreateDialog::colsChanged(int nr_cols)
{
	if (nr_cols != (columns->text()).toInt())
		columns->setValue(nr_cols);
}

 
void QTabularCreateDialog::rowsChanged(int nr_rows)
{
	if (nr_rows != (rows->text()).toInt()) 
		rows->setValue(nr_rows);
}
