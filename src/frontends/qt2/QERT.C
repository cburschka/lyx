/**
 * \file QERT.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QERT.h"
#include "QERTDialog.h"
#include "Qt2BC.h"

#include "controllers/ControlERT.h"

#include <qradiobutton.h>
#include <qpushbutton.h>


namespace lyx {
namespace frontend {

typedef QController<ControlERT, QView<QERTDialog> > base_class;


QERT::QERT(Dialog & parent)
	: base_class(parent, _("TeX Code Settings"))
{
}


void QERT::build_dialog()
{
	dialog_.reset(new QERTDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
}


void QERT::apply()
{
	if (dialog_->openRB->isChecked())
		controller().setStatus(InsetERT::Open);
	else if (dialog_->inlineRB->isChecked())
		controller().setStatus(InsetERT::Inlined);
	else
		controller().setStatus(InsetERT::Collapsed);
}


void QERT::update_contents()
{
	QRadioButton * rb = 0;

	switch (controller().status()) {
		case InsetERT::Open: rb = dialog_->openRB; break;
		case InsetERT::Inlined: rb = dialog_->inlineRB; break;
		case InsetERT::Collapsed: rb = dialog_->collapsedRB; break;
	}

	rb->setChecked(true);
}

} // namespace frontend
} // namespace lyx
