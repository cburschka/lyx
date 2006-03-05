/**
 * \file QErrorListDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QErrorListDialog.h"
#include "QErrorList.h"

#include <q3listbox.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QCloseEvent>

namespace lyx {
namespace frontend {

QErrorListDialog::QErrorListDialog(QErrorList * form)
	: form_(form)
{
	setupUi(this);
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(errorsLB, SIGNAL(returnPressed(Q3ListBoxItem *)),
		form, SLOT(slotClose()));

    connect( errorsLB, SIGNAL( highlighted(int) ), this, SLOT( select_adaptor(int) ) );
}


QErrorListDialog::~QErrorListDialog()
{}


void QErrorListDialog::select_adaptor(int item)
{
	form_->select(item);
}


void QErrorListDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx
