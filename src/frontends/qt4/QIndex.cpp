/**
 * \file QIndex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "ControlCommand.h"
#include "qt_helpers.h"

#include "QIndex.h"
#include "Qt2BC.h"
#include "ButtonController.h"

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QWhatsThis>
#include <QCloseEvent>


using std::string;

/////////////////////////////////////////////////////////////////////
//
// QIndexDialog
//
/////////////////////////////////////////////////////////////////////

namespace lyx {
namespace frontend {

QIndexDialog::QIndexDialog(QIndex * form)
	: form_(form)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));
	connect( keywordED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));

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


/////////////////////////////////////////////////////////////////////
//
// QIndex
//
/////////////////////////////////////////////////////////////////////


typedef QController<ControlCommand, QView<QIndexDialog> > IndexBase;

QIndex::QIndex(Dialog & parent, docstring const & title, QString const & label)
	: IndexBase(parent, title), label_(label)
{
}


void QIndex::build_dialog()
{
	dialog_.reset(new QIndexDialog(this));

	dialog_->keywordLA->setText(label_);

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->keywordED);
}


void QIndex::update_contents()
{
	docstring const contents = controller().params()["name"];
	dialog_->keywordED->setText(toqstr(contents));

	bc().valid(!contents.empty());
}


void QIndex::apply()
{
	controller().params()["name"] = qstring_to_ucs4(dialog_->keywordED->text());
}


bool QIndex::isValid()
{
	return !dialog_->keywordED->text().isEmpty();
}

} // namespace frontend
} // namespace lyx

#include "QIndex_moc.cpp"
