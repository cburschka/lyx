/**
 * \file QIncludeDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QIncludeDialog.h"
#include "QInclude.h"
//Added by qt3to4:
#include <QCloseEvent>
#include "validators.h"

#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>


namespace lyx {
namespace frontend {

QIncludeDialog::QIncludeDialog(QInclude * form)
	: form_(form)
{
	setupUi(this);
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

    connect( visiblespaceCB, SIGNAL( toggled(bool) ), this, SLOT( change_adaptor() ) );
    connect( filenameED, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( loadPB, SIGNAL( clicked() ), this, SLOT( loadClicked() ) );
    connect( browsePB, SIGNAL( clicked() ), this, SLOT( browseClicked() ) );
    connect( typeCO, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );
    connect( typeCO, SIGNAL( activated(int) ), this, SLOT( typeChanged(int) ) );
    connect( previewCB, SIGNAL( toggled(bool) ), this, SLOT( change_adaptor() ) );

	filenameED->setValidator(new PathValidator(true, filenameED));
}


void QIncludeDialog::show()
{
	QDialog::show();
	filenameED->setFocus();
}


void QIncludeDialog::change_adaptor()
{
	form_->changed();
}


void QIncludeDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QIncludeDialog::typeChanged(int v)
{
	switch (v) {
		case 0:
			visiblespaceCB->setEnabled(false);
			visiblespaceCB->setChecked(false);
			previewCB->setEnabled(true);
			break;

		case 1:
			visiblespaceCB->setEnabled(false);
			visiblespaceCB->setChecked(false);
			previewCB->setEnabled(false);
			previewCB->setChecked(false);
			break;

		default:
			visiblespaceCB->setEnabled(true);
			previewCB->setEnabled(false);
			previewCB->setChecked(false);
			break;
	}
}


void QIncludeDialog::loadClicked()
{
	form_->load();
}


void QIncludeDialog::browseClicked()
{
	form_->browse();
}

} // namespace frontend
} // namespace lyx

#include "QIncludeDialog_moc.cpp"
