/**
 * \file QViewSource.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QViewSource.h"
#include "QViewSourceDialog.h"
#include "qt_helpers.h"

#include "frontends/Application.h"

#include "controllers/ControlViewSource.h"

#include <sstream>

#include <qtextview.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

namespace lyx {
namespace frontend {

typedef QController<ControlViewSource, QView<QViewSourceDialog> > base_class;

QViewSource::QViewSource(Dialog & parent)
	: base_class(parent, lyx::docstring())
{}


void QViewSource::build_dialog()
{
	dialog_.reset(new QViewSourceDialog(this));
	dialog_->viewSourceTV->setReadOnly(true);
	dialog_->viewSourceTV->setTextFormat(Qt::PlainText);
	// this is personal. I think source code should be in fixed-size font
	QFont font(toqstr(theApp->typewriterFontName()));
	font.setFixedPitch(true);
	font.setStyleHint(QFont::TypeWriter);
	dialog_->viewSourceTV->setFont(font);
	// again, personal taste
	dialog_->viewSourceTV->setWordWrap(QTextEdit::NoWrap);
}


void QViewSource::update_source()
{
	bool fullSource = dialog_->viewFullSourceCB->isChecked();
	dialog_->viewSourceTV->setText(toqstr(controller().updateContent(fullSource)));
}


void QViewSource::update_contents()
{
	setTitle(controller().title());
	if (dialog_->autoUpdateCB->isChecked())
		update_source();
}

} // namespace frontend
} // namespace lyx
