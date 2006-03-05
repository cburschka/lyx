/**
 * \file QSendtoDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QSendtoDialog.h"
#include "QSendto.h"

#include <qpushbutton.h>
//Added by qt3to4:
#include <QCloseEvent>


namespace lyx {
namespace frontend {

QSendtoDialog::QSendtoDialog(QSendto * form)
	: form_(form)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

    connect( formatLB, SIGNAL( highlighted(const QString&) ), this, SLOT( slotFormatHighlighted(const QString&) ) );
    connect( formatLB, SIGNAL( selected(const QString&) ), this, SLOT( slotFormatSelected(const QString&) ) );
    connect( formatLB, SIGNAL( highlighted(const QString&) ), this, SLOT( changed_adaptor() ) );
    connect( commandCO, SIGNAL( textChanged(const QString&) ), this, SLOT( changed_adaptor() ) );
}


void QSendtoDialog::changed_adaptor()
{
	form_->changed();
}


void QSendtoDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx
