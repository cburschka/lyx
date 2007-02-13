/**
 * \file QIndexDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "qt_helpers.h"

#include "QIndex.h"
#include "QIndexDialog.h"

#include <QPushButton>
#include <QLineEdit>
#include <QWhatsThis>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

QIndexDialog::QIndexDialog(QIndex * form)
	: form_(form)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect( keywordED, SIGNAL( textChanged(const QString&) ), 
		this, SLOT( change_adaptor() ) );

	setFocusProxy(keywordED);

	keywordED->setWhatsThis( qt_(
		"The format of the entry in the index.\n"
		"\n"
		"An entry can be specified as a sub-entry of\n"
		"another with \"!\":\n"
		"\n"
		"cars!mileage\n"
		"\n"
		"You can cross-refer to another entry like so:\n"
		"\n"
		"cars!mileage|see{economy}\n"
		"\n"
		"For further details refer to the local LaTeX\n"
		"documentation.\n")
	);
}


void QIndexDialog::change_adaptor()
{
	form_->changed();
}


void QIndexDialog::reject()
{
	form_->slotClose();
}


void QIndexDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx

#include "QIndexDialog_moc.cpp"
