/**
 * \file QThesaurus.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "ControlThesaurus.h"
#include "QThesaurusDialog.h"
#include "QThesaurus.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistbox.h>

typedef QController<ControlThesaurus, QView<QThesaurusDialog> > base_class;


QThesaurus::QThesaurus(Dialog & parent)
	: base_class(parent, _("LyX: Thesaurus"))
{
}


void QThesaurus::build_dialog()
{
	dialog_.reset(new QThesaurusDialog(this));

	bcview().setCancel(dialog_->closePB);
	bcview().setApply(dialog_->replacePB);
	bcview().addReadOnly(dialog_->replaceED);
	bcview().addReadOnly(dialog_->replacePB);
}


void QThesaurus::update_contents()
{
	dialog_->entryED->setText(toqstr(controller().text()));
	dialog_->replaceED->setText("");
	dialog_->updateLists();
}


void QThesaurus::replace()
{
	controller().replace(fromqstr(dialog_->replaceED->text()));
}
