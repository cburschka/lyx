/**
 * \file QIndex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "ControlCommand.h"
#include "qt_helpers.h"

#include "QIndexDialog.h"
#include "QIndex.h"
#include "Qt2BC.h"
#include "ButtonController.h"
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>

typedef QController<ControlCommand, QView<QIndexDialog> > base_class;


QIndex::QIndex(Dialog & parent, string const & title, QString const & label)
	: base_class(parent, title), label_(label)
{
}


void QIndex::build_dialog()
{
	dialog_.reset(new QIndexDialog(this));

	dialog_->keywordLA->setText(label_);

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->keywordED);
}


void QIndex::update_contents()
{
	string const contents = controller().params().getContents();
	dialog_->keywordED->setText(toqstr(contents));

	bc().valid(!contents.empty());
}


void QIndex::apply()
{
	controller().params().setContents(fromqstr(dialog_->keywordED->text()));
}


bool QIndex::isValid()
{
	return !dialog_->keywordED->text().isEmpty();
}
