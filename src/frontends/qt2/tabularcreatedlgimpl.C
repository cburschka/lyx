/**
 * \file tabularcreatedlgimpl.C
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include "tabularcreatedlg.h"

#include <config.h>
#include <gettext.h>
#include <string>

#include "tabularcreatedlgimpl.h"
#include "FormTabularCreate.h"
#include "support/lstrings.h"

#include "qpushbutton.h"
#include "qspinbox.h"
#include "emptytable.h"
#include "qpainter.h"
#include "qtableview.h"
#include "qtooltip.h"

TabularCreateDlgImpl::TabularCreateDlgImpl(FormTabularCreate* form, QWidget* parent,  const char* name, bool modal, WFlags fl )
    : InsertTabularDlg( parent, name, modal, fl ), form_(form)
{
   setCaption(name);
   table->setMinimumSize(100,100);
   rows->setValue(5);
   columns->setValue(5);
   QToolTip::add(table, _("Drag with left mouse button to resize"));
}

TabularCreateDlgImpl::~TabularCreateDlgImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

void TabularCreateDlgImpl::insert_tabular()
{
   form_->apply((rows->text()).toInt(), (columns->text()).toInt());
   form_->close();
   hide();
}

void TabularCreateDlgImpl::cancel_adaptor()
{
   form_->close();
   hide();
}

void TabularCreateDlgImpl::colsChanged(int nr_cols)
{
   if (nr_cols != (columns->text()).toInt())
     columns->setValue(nr_cols);
}

void TabularCreateDlgImpl::rowsChanged(int nr_rows)
{
   if (nr_rows != (rows->text()).toInt()) 
     rows->setValue(nr_rows);
}

