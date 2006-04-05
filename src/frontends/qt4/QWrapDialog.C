/**
 * \file QWrapDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QWrapDialog.h"
#include "QWrap.h"

#include <qpushbutton.h>
//Added by qt3to4:
#include <QCloseEvent>

namespace lyx {
namespace frontend {


QWrapDialog::QWrapDialog(QWrap * form)
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

    connect( widthED, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( unitsLC, SIGNAL( selectionChanged(LyXLength::UNIT) ), this, SLOT( change_adaptor() ) );
    connect( valignCO, SIGNAL( highlighted(const QString&) ), this, SLOT( change_adaptor() ) );
}


void QWrapDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QWrapDialog::change_adaptor()
{
	form_->changed();
}

} // namespace frontend
} // namespace lyx
