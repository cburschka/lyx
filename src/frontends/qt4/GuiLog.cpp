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

#include "frontends/Clipboard.h"

#include "support/docstring.h"
#include "support/FileName.h"
#include "support/gettext.h"

#include <QDesktopServices>
#include <QTextBrowser>
#include <QSyntaxHighlighter>
#include <QUrl>
#include <QClipboard>

#include <fstream>
#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


// Regular expressions needed at several places
// FIXME: These regexes are incomplete. It would be good if we could collect those used in LaTeX::scanLogFile
//        and LaTeX::scanBlgFile and re-use them here!(spitz, 2013-05-27)
// Information
QRegExp exprInfo("^(Document Class:|LaTeX Font Info:|File:|Package:|Language:|Underfull|Overfull|.*> INFO - |\\(|\\\\).*$");
// Warnings
QRegExp exprWarning("^(LaTeX Warning|LaTeX Font Warning|Package [\\w\\.]+ Warning|Class \\w+ Warning|Warning--|.*> WARN - ).*$");
// Errors
QRegExp exprError("^(!|.*---line [0-9]+ of file|.*> FATAL - |.*> ERROR - |Missing character: There is no ).*$");


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
	connect(logTypeCO, SIGNAL(activated(int)),
		this, SLOT(typeChanged(int)));

	bc().setPolicy(ButtonPolicy::OkCancelPolicy);

	// set syntax highlighting
	highlighter = new LogHighlighter(logTB->document());

	logTB->setReadOnly(true);
	QFont font(guiApp->typewriterFontName());
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


void GuiLog::typeChanged(int i)
{
	string const type =
		fromqstr(logTypeCO->itemData(i).toString());
	string ext;
	if (type == "latex")
		ext = "log";
	else if (type == "bibtex")
		ext = "blg";
	else if (type == "index")
		ext = "ilg";

	if (!ext.empty())
		logfile_.changeExtension(ext);

	updateContents();
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


void GuiLog::on_openDirPB_clicked()
{
	support::FileName dir = logfile_.onlyPath();
	if (!dir.exists())
		return;
	QUrl qdir(QUrl::fromLocalFile(toqstr(from_utf8(dir.absFileName()))));
	// Give hints in case of bugs
	if (!qdir.isValid()) {
		LYXERR0("QUrl is invalid!");
		return;
	}
	if (!QDesktopServices::openUrl(qdir))
		LYXERR0("Unable to open QUrl even though dir exists!");
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

	logTypeCO->setEnabled(logtype == "latex");
	logTypeCO->clear();
	
	FileName log(logfile);
	
	if (logtype == "latex") {
		type_ = LatexLog;
		logTypeCO->addItem(qt_("LaTeX"), toqstr(logtype));
		FileName tmp = log;
		tmp.changeExtension("blg");
		if (tmp.exists())
			logTypeCO->addItem(qt_("BibTeX"), QString("bibtex"));
		tmp.changeExtension("ilg");
		if (tmp.exists())
			logTypeCO->addItem(qt_("Index"), QString("index"));
	// FIXME: not sure "literate" still works.
	} else if (logtype == "literate") {
		type_ = LiterateLog;
		logTypeCO->addItem(qt_("Literate"), toqstr(logtype));
	} else if (logtype == "lyx2lyx") {
		type_ = Lyx2lyxLog;
		logTypeCO->addItem(qt_("LyX2LyX"), toqstr(logtype));
	} else if (logtype == "vc") {
		type_ = VCLog;
		logTypeCO->addItem(qt_("Version Control"), toqstr(logtype));
	} else
		return false;

	logfile_ = log;

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
		ss << to_utf8(_("Log file not found."));
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


Dialog * createGuiLog(GuiView & lv) { return new GuiLog(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiLog.cpp"
