/**
 * \file QERT.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlERT.h"
#include "gettext.h"

#include <qradiobutton.h>
#include <qpushbutton.h>

#include "QERTDialog.h"
#include "QERT.h"
#include "Qt2BC.h"

typedef Qt2CB<ControlERT, Qt2DB<QERTDialog> > base_class;

QERT::QERT()
	: base_class(_("LaTeX ERT"))
{
}


void QERT::build_dialog()
{
	dialog_.reset(new QERTDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
}


void QERT::apply()
{
	if (dialog_->openRB->isChecked())
		controller().params().status = InsetERT::Open;
	else if (dialog_->inlineRB->isChecked())
		controller().params().status = InsetERT::Inlined;
	else
		controller().params().status = InsetERT::Collapsed;
}


void QERT::update_contents()
{
	QRadioButton * rb;

	switch (controller().params().status) {
		case InsetERT::Open: rb = dialog_->openRB; break;
		case InsetERT::Inlined: rb = dialog_->inlineRB; break;
		case InsetERT::Collapsed: rb = dialog_->collapsedRB; break;
	}

	rb->setChecked(true);
}
