/**
 * \file QNomenclDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author O. U. Baran
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "qt_helpers.h"

#include "QNomencl.h"
#include "QNomenclDialog.h"

#include <QPushButton>
#include <QLineEdit>
#include <QWhatsThis>
#include <QCloseEvent>
#include <QTextEdit>

namespace lyx {
namespace frontend {

QNomenclDialog::QNomenclDialog(QNomencl * form)
	: form_(form)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));
	connect(symbolED, SIGNAL(textChanged(const QString&)),
	        this, SLOT(change_adaptor()));
	connect(descriptionTE, SIGNAL(textChanged()),
	        this, SLOT(change_adaptor()));

	setFocusProxy(descriptionTE);
}


void QNomenclDialog::show()
{
	QDialog::show();
}


void QNomenclDialog::change_adaptor()
{
	form_->changed();
}


void QNomenclDialog::reject()
{
	form_->slotClose();
}


void QNomenclDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx

#include "QNomenclDialog_moc.cpp"
