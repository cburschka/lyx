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

typedef Qt2CB<ControlThesaurus, Qt2DB<QThesaurusDialog> > base_class;


QThesaurus::QThesaurus()
	: base_class(qt_("Thesaurus"))
{
}


void QThesaurus::build_dialog()
{
	dialog_.reset(new QThesaurusDialog(this));

	bc().setCancel(dialog_->closePB);
	bc().setApply(dialog_->replacePB);
	bc().addReadOnly(dialog_->replaceED);
	bc().addReadOnly(dialog_->replacePB);
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
