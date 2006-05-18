/**
 * \file QERTDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QERTDialog.h"
#include "QERT.h"

#include <qpushbutton.h>
//Added by qt3to4:
#include <QCloseEvent>

namespace lyx {
namespace frontend {

QERTDialog::QERTDialog(QERT * form)
	: form_(form)
{
	setupUi(this);
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

    connect( inlineRB, SIGNAL( toggled(bool) ), this, SLOT( change_adaptor() ) );
    connect( collapsedRB, SIGNAL( toggled(bool) ), this, SLOT( change_adaptor() ) );
    connect( openRB, SIGNAL( toggled(bool) ), this, SLOT( change_adaptor() ) );
}


void QERTDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QERTDialog::change_adaptor()
{
	form_->changed();
}

} // namespace frontend
} // namespace lyx

#include "QERTDialog_moc.cpp"
