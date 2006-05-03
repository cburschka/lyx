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

#include <QListWidget>
#include <QPushButton>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

QErrorListDialog::QErrorListDialog(QErrorList * form)
	: form_(form)
{
	setupUi(this);
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(errorsLW, SIGNAL( itemActivated(QListWidgetItem *)),
		form, SLOT(slotClose()));
	connect( errorsLW, SIGNAL( itemClicked(QListWidgetItem *) ), 
		this, SLOT( select_adaptor(QListWidgetItem *) ) );
}


QErrorListDialog::~QErrorListDialog()
{}


void QErrorListDialog::select_adaptor(QListWidgetItem * item)
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
