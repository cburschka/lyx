/**
 * \file GuiLog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiLog.h"

#include "GuiApplication.h"
#include "qt_helpers.h"
#include "Lexer.h"

#include "support/docstring.h"
#include "support/gettext.h"

#include <QTextBrowser>
#include <QSyntaxHighlighter>
#include <QClipboard>

#include <fstream>
#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


// Regular expressions needed at several places
// Information
QRegExp exprInfo("^(Document Class:|LaTeX Font Info:|File:|Package:|Language:|Underfull|Overfull|\\(|\\\\).*$");
// Warnings
QRegExp exprWarning("^(LaTeX Warning|LaTeX Font Warning|Package [\\w\\.]+ Warning|Class \\w+ Warning).*$");
// Errors
QRegExp exprError("^!.*$");


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
	int index = exprInfo.indexIn(text);
	while (index >= 0) {
		int length = exprInfo.matchedLength();
		setFormat(index, length, infoFormat);
		index = exprInfo.indexIn(text, index + length);
	}
	// LaTeX Warning:
	index = exprWarning.indexIn(text);
	while (index >= 0) {
		int length = exprWarning.matchedLength();
		setFormat(index, length, warningFormat);
		index = exprWarning.indexIn(text, index + length);
	}
	// ! error
	index = exprError.indexIn(text);
	while (index >= 0) {
		int length = exprError.matchedLength();
		setFormat(index, length, errorFormat);
		index = exprError.indexIn(text, index + length);
	}
}


/////////////////////////////////////////////////////////////////////
//
// GuiLog
//
/////////////////////////////////////////////////////////////////////

GuiLog::GuiLog(GuiView & lv)
	: GuiDialog(lv, "log", qt_("LaTeX Log")), type_(LatexLog)
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(updatePB, SIGNAL(clicked()), this, SLOT(updateContents()));
	connect(findPB, SIGNAL(clicked()), this, SLOT(find()));
	// FIXME: find via returnPressed() does not work!
	connect(findLE, SIGNAL(returnPressed()), this, SLOT(find()));

	bc().setPolicy(ButtonPolicy::OkCancelPolicy);

	// set syntax highlighting
	highlighter = new LogHighlighter(logTB->document());

	logTB->setReadOnly(true);
	QFont font(guiApp->typewriterFontName());
	font.setKerning(false);
	font.setFixedPitch(true);
	font.setStyleHint(QFont::TypeWriter);
	logTB->setFont(font);
}


void GuiLog::updateContents()
{
	setTitle(toqstr(title()));

	ostringstream ss;
	getContents(ss);

	logTB->setPlainText(toqstr(ss.str()));

	nextErrorPB->setEnabled(contains(exprError));
	nextWarningPB->setEnabled(contains(exprWarning));
}


void GuiLog::find()
{
	logTB->find(findLE->text());
}


void GuiLog::on_nextErrorPB_clicked()
{
	goTo(exprError);
}


void GuiLog::on_nextWarningPB_clicked()
{
	goTo(exprWarning);
}


void GuiLog::goTo(QRegExp const & exp) const
{
	QTextCursor const newc =
		logTB->document()->find(exp, logTB->textCursor());
	logTB->setTextCursor(newc);
}


bool GuiLog::contains(QRegExp const & exp) const
{
	return !logTB->document()->find(exp, logTB->textCursor()).isNull();
}


bool GuiLog::initialiseParams(string const & data)
{
	istringstream is(data);
	Lexer lex;
	lex.setStream(is);

	string logtype, logfile;
	lex >> logtype;
	if (lex) {
		lex.next(true);
		logfile = lex.getString();
	}
	if (!lex)
		// Parsing of the data failed.
		return false;

	if (logtype == "latex")
		type_ = LatexLog;
	else if (logtype == "literate")
		type_ = LiterateLog;
	else if (logtype == "lyx2lyx")
		type_ = Lyx2lyxLog;
	else if (logtype == "vc")
		type_ = VCLog;
	else
		return false;

	logfile_ = FileName(logfile);

	updateContents();

	return true;
}


void GuiLog::clearParams()
{
	logfile_.erase();
}


docstring GuiLog::title() const
{
	switch (type_) {
	case LatexLog:
		return _("LaTeX Log");
	case LiterateLog:
		return _("Literate Programming Build Log");
	case Lyx2lyxLog:
		return _("lyx2lyx Error Log");
	case VCLog:
		return _("Version Control Log");
	default:
		return docstring();
	}
}


void GuiLog::getContents(ostream & ss) const
{
	ifstream in(logfile_.toFilesystemEncoding().c_str());

	bool success = false;

	// FIXME UNICODE
	// Our caller interprets the file contents as UTF8, but is that
	// correct?
	if (in) {
		ss << in.rdbuf();
		success = ss.good();
	}

	if (success)
		return;

	switch (type_) {
	case LatexLog:
		ss << to_utf8(_("No LaTeX log file found."));
		break;
	case LiterateLog:
		ss << to_utf8(_("No literate programming build log file found."));
		break;
	case Lyx2lyxLog:
		ss << to_utf8(_("No lyx2lyx error log file found."));
		break;
	case VCLog:
		ss << to_utf8(_("No version control log file found."));
		break;
	}
}


void GuiLog::on_copyPB_clicked()
{
	qApp->clipboard()->setText(logTB->toPlainText());
}


Dialog * createGuiLog(GuiView & lv) { return new GuiLog(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiLog_moc.cpp"
