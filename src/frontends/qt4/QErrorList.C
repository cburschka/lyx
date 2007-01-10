/**
 * \file QErrorList.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QErrorList.h"
#include "QErrorListDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "controllers/ControlErrorList.h"

#include <QListWidget>
#include <QTextBrowser>
#include <QPushButton>

namespace lyx {
namespace frontend {

typedef QController<ControlErrorList, QView<QErrorListDialog> > base_class;

QErrorList::QErrorList(Dialog & parent)
	: base_class(parent, lyx::docstring())
{}


void QErrorList::build_dialog()
{
	dialog_.reset(new QErrorListDialog(this));
	bcview().setCancel(dialog_->closePB);
}


void QErrorList::select(QListWidgetItem * wi)
{
	int const item = dialog_->errorsLW->row(wi);
	controller().goTo(item);
	dialog_->descriptionTB->setPlainText(toqstr(controller().errorList()[item].description));
}


void QErrorList::update_contents()
{
	setTitle(lyx::from_utf8(controller().name()));
	dialog_->errorsLW->clear();
	dialog_->descriptionTB->setPlainText(QString());

	ErrorList::const_iterator it = controller().errorList().begin();
	ErrorList::const_iterator end = controller().errorList().end();
	for(; it != end; ++it) {
		dialog_->errorsLW->addItem(toqstr(it->error));
	}

	dialog_->errorsLW->setCurrentRow(0);
	select(dialog_->errorsLW->item(0));
}

} // namespace frontend
} // namespace lyx
