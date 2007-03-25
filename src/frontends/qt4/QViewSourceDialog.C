/**
 * \file QViewSourceDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Bo Peng
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QViewSourceDialog.h"

#include "QViewSource.h"


namespace lyx {
namespace frontend {

QViewSourceDialog::QViewSourceDialog(QViewSource * form)
	: form_(form)
{
	setupUi(this);

	connect(viewFullSourceCB, SIGNAL(clicked()),
		this, SLOT(update()));
	connect(autoUpdateCB, SIGNAL(toggled(bool)),
		updatePB, SLOT(setDisabled(bool)));
	connect(updatePB, SIGNAL(clicked()),
		this, SLOT(update()));

	// setting a document at this point trigger an assertion in Qt
	// so we disable the signals here:
	form_->document()->blockSignals(true);
	viewSourceTV->setDocument(form_->document());
	form_->document()->blockSignals(false);
	viewSourceTV->setReadOnly(true);
	///dialog_->viewSourceTV->setAcceptRichText(false);
	// this is personal. I think source code should be in fixed-size font
	QFont font(toqstr(theApp()->typewriterFontName()));
	font.setKerning(false);
	font.setFixedPitch(true);
	font.setStyleHint(QFont::TypeWriter);
	viewSourceTV->setFont(font);
	// again, personal taste
	viewSourceTV->setWordWrapMode(QTextOption::NoWrap);
}


void QViewSourceDialog::update()
{
	if (autoUpdateCB->isChecked())
		form_->update(viewFullSourceCB->isChecked());

	QWidget::update();
}

} // namespace frontend
} // namespace lyx

#include "QViewSourceDialog_moc.cpp"
