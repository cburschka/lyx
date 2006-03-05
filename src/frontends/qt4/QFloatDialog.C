/**
 * \file QFloatDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QFloatDialog.h"
#include "QFloat.h"
//Added by qt3to4:
#include <QCloseEvent>
#include "floatplacement.h"

#include <qpushbutton.h>


namespace lyx {
namespace frontend {

QFloatDialog::QFloatDialog(QFloat * form)
	: form_(form)
{
	setupUi(this);
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

	// enable span columns checkbox
	floatFP->useWide();

	// enable sideways checkbox
	floatFP->useSideways();

	connect(floatFP, SIGNAL(changed()),
		this, SLOT(change_adaptor()));
}


void QFloatDialog::change_adaptor()
{
	form_->changed();
}


void QFloatDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx
