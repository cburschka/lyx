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

#include "GuiLog.h"

#include "ControlLog.h"
#include "qt_helpers.h"

#include "frontends/Application.h"

#include <QCloseEvent>
#include <QTextBrowser>
#include <QSyntaxHighlighter>

#include <sstream>


namespace lyx {
namespace frontend {


/////////////////////////////////////////////////////////////////////
//
// LogHighlighter
//
////////////////////////////////////////////////////////////////////

class LogHighlighter : public QSyntaxHighlighter
{
public:
	LogHighlighter(QTextDocument * parent);

private:
	void highlightBlock(QString const & text);

private:
	QTextCharFormat infoFormat;
	QTextCharFormat warningFormat;
	QTextCharFormat errorFormat;
};



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

GuiLogDialog::GuiLogDialog(LyXView & lv)
	: GuiDialog(lv, "log")
{
	setupUi(this);
	setController(new ControlLog(*this));

	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(updatePB, SIGNAL(clicked()), this, SLOT(updateClicked()));

	bc().setPolicy(ButtonPolicy::OkCancelPolicy);

	// set syntax highlighting
	highlighter = new LogHighlighter(logTB->document());

	logTB->setReadOnly(true);
	QFont font(toqstr(theApp()->typewriterFontName()));
	font.setKerning(false);
	font.setFixedPitch(true);
	font.setStyleHint(QFont::TypeWriter);
	logTB->setFont(font);
}


ControlLog & GuiLogDialog::controller() const
{
	return static_cast<ControlLog &>(GuiDialog::controller());
}


void GuiLogDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiLogDialog::updateClicked()
{
	update_contents();
}


void GuiLogDialog::update_contents()
{
	setViewTitle(controller().title());

	std::ostringstream ss;
	controller().getContents(ss);

	logTB->setPlainText(toqstr(ss.str()));
}

} // namespace frontend
} // namespace lyx

#include "GuiLog_moc.cpp"
