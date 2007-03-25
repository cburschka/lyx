/**
 * \file QViewSource.C
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

#include "QViewSource.h"
#include "QViewSourceDialog.h"
#include "qt_helpers.h"

namespace lyx {
namespace frontend {


latexHighlighter::latexHighlighter(QTextDocument * parent) :
	QSyntaxHighlighter(parent)
{
	keywordFormat.setForeground(Qt::darkBlue);
	keywordFormat.setFontWeight(QFont::Bold);
	commentFormat.setForeground(Qt::gray);
	mathFormat.setForeground(Qt::red);
}


void latexHighlighter::highlightBlock(QString const & text)
{
	// comment
	QRegExp exprComment("^%.*$");
	int index = text.indexOf(exprComment);
	while (index >= 0) {
		int length = exprComment.matchedLength();
		setFormat(index, length, commentFormat);
		index = text.indexOf(exprComment, index + length);
	}
	// $ $ 
	QRegExp exprMath("\\$[^\\$]*\\$");
	index = text.indexOf(exprMath);
	while (index >= 0) {
		int length = exprMath.matchedLength();
		setFormat(index, length, mathFormat);
		index = text.indexOf(exprMath, index + length);
	}
	// [ ]
	QRegExp exprDispMath("\\[[^\\]]*\\]");
	index = text.indexOf(exprDispMath);
	while (index >= 0) {
		int length = exprDispMath.matchedLength();
		setFormat(index, length, mathFormat);
		index = text.indexOf(exprDispMath, index + length);
	}
	// \whatever
	QRegExp exprKeyword("\\\\[A-Za-z]+");
	index = text.indexOf(exprKeyword);
	while (index >= 0) {
		int length = exprKeyword.matchedLength();
		setFormat(index, length, keywordFormat);
		index = text.indexOf(exprKeyword, index + length);
	}
}


QViewSource::QViewSource(Dialog & parent)
	: ControlViewSource(parent)
{
	document_ = new QTextDocument(this);
	// set syntex highlighting
	highlighter_ = new latexHighlighter(document_);
}


void QViewSource::update(bool full_source)
{
	document_->setPlainText(toqstr(updateContent(full_source)));
}


} // namespace frontend
} // namespace lyx

#include "QViewSource_moc.cpp"
