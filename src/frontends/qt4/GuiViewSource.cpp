/**
 * \file GuiViewSource.cpp
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

#include "GuiViewSource.h"
#include "qt_helpers.h"

#include <boost/tuple/tuple.hpp>

#include <QTextCursor>
#include <QTextDocument>

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// GuiViewSourceDialog
//
/////////////////////////////////////////////////////////////////////

GuiViewSourceDialog::GuiViewSourceDialog(GuiViewSource * form)
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


void GuiViewSourceDialog::update()
{
	if (autoUpdateCB->isChecked())
		form_->update(viewFullSourceCB->isChecked());

	int beg, end;
	boost::tie(beg, end) = form_->getRows();
	QTextCursor c = QTextCursor(viewSourceTV->document());
	c.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, beg);
	c.select(QTextCursor::BlockUnderCursor);
	c.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, end - beg + 1);
	viewSourceTV->setTextCursor(c);
	QWidget::update();
}


/////////////////////////////////////////////////////////////////////
//
// LaTeXHighlighter
//
/////////////////////////////////////////////////////////////////////


LaTeXHighlighter::LaTeXHighlighter(QTextDocument * parent)
	: QSyntaxHighlighter(parent)
{
	keywordFormat.setForeground(Qt::darkBlue);
	keywordFormat.setFontWeight(QFont::Bold);
	commentFormat.setForeground(Qt::darkGray);
	mathFormat.setForeground(Qt::red);
}


void LaTeXHighlighter::highlightBlock(QString const & text)
{
	// $ $
	QRegExp exprMath("\\$[^\\$]*\\$");
	int index = text.indexOf(exprMath);
	while (index >= 0) {
		int length = exprMath.matchedLength();
		setFormat(index, length, mathFormat);
		index = text.indexOf(exprMath, index + length);
	}
	// [ ]
	QRegExp exprStartDispMath("(\\\\\\[|"
		"\\\\begin\\{equation\\**\\}|"
		"\\\\begin\\{eqnarray\\**\\}|"
		"\\\\begin\\{align(ed|at)*\\**\\}|"
		"\\\\begin\\{flalign\\**\\}|"
		"\\\\begin\\{gather\\**\\}|"
		"\\\\begin\\{multline\\**\\}|"
		"\\\\begin\\{array\\**\\}|"
		"\\\\begin\\{cases\\**\\}"
		")");
	QRegExp exprEndDispMath("(\\\\\\]|"
		"\\\\end\\{equation\\**\\}|"
		"\\\\end\\{eqnarray\\**\\}|"
		"\\\\end\\{align(ed|at)*\\**\\}|"
		"\\\\end\\{flalign\\**\\}|"
		"\\\\end\\{gather\\**\\}|"
		"\\\\end\\{multline\\**\\}|"
		"\\\\end\\{array\\**\\}|"
		"\\\\end\\{cases\\**\\}"
		")");
	int startIndex = 0;
	// if previous block was in 'disp math'
	// start search from 0 (for end disp math)
	// otherwise, start search from 'begin disp math'
	if (previousBlockState() != 1)
		startIndex = text.indexOf(exprStartDispMath);
	while (startIndex >= 0) {
		int endIndex = text.indexOf(exprEndDispMath, startIndex);
		int length;
		if (endIndex == -1) {
			setCurrentBlockState(1);
			length = text.length() - startIndex;
		} else {
			length = endIndex - startIndex + exprEndDispMath.matchedLength();
		}
		setFormat(startIndex, length, mathFormat);
		startIndex = text.indexOf(exprStartDispMath, startIndex + length);
	}
	// \whatever
	QRegExp exprKeyword("\\\\[A-Za-z]+");
	index = text.indexOf(exprKeyword);
	while (index >= 0) {
		int length = exprKeyword.matchedLength();
		setFormat(index, length, keywordFormat);
		index = text.indexOf(exprKeyword, index + length);
	}
	// comment
	QRegExp exprComment("(^|[^\\\\])%.*$");
	index = text.indexOf(exprComment);
	while (index >= 0) {
		int const length = exprComment.matchedLength();
		setFormat(index, length, commentFormat);
		index = text.indexOf(exprComment, index + length);
	}
}


GuiViewSource::GuiViewSource(Dialog & parent)
	: ControlViewSource(parent)
{
	document_ = new QTextDocument(this);
	highlighter_ = new LaTeXHighlighter(document_);
}


/////////////////////////////////////////////////////////////////////
//
// GuiViewSource
//
/////////////////////////////////////////////////////////////////////

void GuiViewSource::update(bool full_source)
{
	document_->setPlainText(toqstr(updateContent(full_source)));
}


} // namespace frontend
} // namespace lyx

#include "GuiViewSource_moc.cpp"
