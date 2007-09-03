/**
 * \file GuiLog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include <sstream>

#include "GuiLog.h"
#include "qt_helpers.h"

#include "frontends/Application.h"

#include <QCloseEvent>
#include <QTextBrowser>
#include <QTextBrowser>

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// GuiLogDialog
//
/////////////////////////////////////////////////////////////////////


GuiLogDialog::GuiLogDialog(GuiLog * form)
	: form_(form)
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect( updatePB, SIGNAL( clicked() ),
		this, SLOT( updateClicked() ) );
}


void GuiLogDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void GuiLogDialog::updateClicked()
{
	form_->update_contents();
}


/////////////////////////////////////////////////////////////////////
//
// LogHighlighter
//
/////////////////////////////////////////////////////////////////////

LogHighlighter::LogHighlighter(QTextDocument * parent)
	: QSyntaxHighlighter(parent)
{
	infoFormat.setForeground(Qt::darkGray);
	warningFormat.setForeground(Qt::darkBlue);
	errorFormat.setForeground(Qt::red);
}


void LogHighlighter::highlightBlock(QString const & text)
{
	// Info
	QRegExp exprInfo("^(Document Class:|LaTeX Font Info:|File:|Package:|Language:|Underfull|Overfull|\\(|\\\\).*$");
	int index = text.indexOf(exprInfo);
	while (index >= 0) {
		int length = exprInfo.matchedLength();
		setFormat(index, length, infoFormat);
		index = text.indexOf(exprInfo, index + length);
	}
	// LaTeX Warning:
	QRegExp exprWarning("^LaTeX Warning.*$");
	index = text.indexOf(exprWarning);
	while (index >= 0) {
		int length = exprWarning.matchedLength();
		setFormat(index, length, warningFormat);
		index = text.indexOf(exprWarning, index + length);
	}
	// ! error
	QRegExp exprError("^!.*$");
	index = text.indexOf(exprError);
	while (index >= 0) {
		int length = exprError.matchedLength();
		setFormat(index, length, errorFormat);
		index = text.indexOf(exprError, index + length);
	}
}


/////////////////////////////////////////////////////////////////////
//
// GuiLog
//
/////////////////////////////////////////////////////////////////////


GuiLog::GuiLog(GuiDialog & parent)
	:  GuiView<GuiLogDialog>(parent, docstring())
{}


void GuiLog::build_dialog()
{
	dialog_.reset(new GuiLogDialog(this));
	// set syntax highlighting
	highlighter = new LogHighlighter(dialog_->logTB->document());
	//
	dialog_->logTB->setReadOnly(true);
	QFont font(toqstr(theApp()->typewriterFontName()));
	font.setKerning(false);
	font.setFixedPitch(true);
	font.setStyleHint(QFont::TypeWriter);
	dialog_->logTB->setFont(font);
}


void GuiLog::update_contents()
{
	setViewTitle(controller().title());

	std::ostringstream ss;
	controller().getContents(ss);

	dialog_->logTB->setPlainText(toqstr(ss.str()));
}

} // namespace frontend
} // namespace lyx

#include "GuiLog_moc.cpp"
