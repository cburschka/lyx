/**
 * \file QFloat.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "ControlFloat.h"
#include "QFloatDialog.h"
#include "QFloat.h"
#include "Qt2BC.h"
#include "floatplacement.h"

#include "insets/insetfloat.h"

#include <qpushbutton.h>

typedef QController<ControlFloat, QView<QFloatDialog> > base_class;


QFloat::QFloat(Dialog & parent)
	: base_class(parent, _("LyX: Float Settings"))
{
}


void QFloat::build_dialog()
{
	dialog_.reset(new QFloatDialog(this));

	bcview().setCancel(dialog_->closePB);
	bcview().setApply(dialog_->applyPB);
	bcview().setOK(dialog_->okPB);
	bcview().setRestore(dialog_->restorePB);

	bcview().addReadOnly(dialog_->floatFP);
}


void QFloat::update_contents()
{
	dialog_->floatFP->set(controller().params());
}


void QFloat::apply()
{
	InsetFloatParams & params = controller().params();

	params.placement = dialog_->floatFP->get(params.wide, params.sideways);
}
