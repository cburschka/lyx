/**
 * \file QBibitemDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QBibitemDialog.h"

#include <qpushbutton.h>
//Added by qt3to4:
#include <QCloseEvent>

namespace lyx {
namespace frontend {

QBibitemDialog::QBibitemDialog(QBibitem * form)
	: form_(form)
{
	setupUi(this);
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

    connect( keyED, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( labelED, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
}


void QBibitemDialog::change_adaptor()
{
	form_->changed();
}


void QBibitemDialog::closeEvent(QCloseEvent *e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx
