/**
 * \file QThesaurus.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlThesaurus.h"
#include "QThesaurusDialog.h"
#include "QThesaurus.h"
#include "Qt2BC.h"
#include "gettext.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistbox.h>

typedef Qt2CB<ControlThesaurus, Qt2DB<QThesaurusDialog> > base_class;
 
QThesaurus::QThesaurus(ControlThesaurus & c)
	: base_class(c, _("Thesaurus"))
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
	dialog_->entryED->setText(controller().text().c_str());
	dialog_->updateLists(); 
}

 
void QThesaurus::replace()
{
	controller().replace(dialog_->replaceED->text().latin1());
}
