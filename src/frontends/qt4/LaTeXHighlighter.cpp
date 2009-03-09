/**
 * \file LaTeXHighlighter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LaTeXHighlighter.h"
#include "qt_helpers.h"

#include "support/lassert.h"

#include <QString>
#include <QTextDocument>


using namespace lyx::support;

namespace lyx {
namespace frontend {

LaTeXHighlighter::LaTeXHighlighter(QTextDocument * parent)
	: QSyntaxHighlighter(parent)
{
	keywordFormat.setForeground(Qt::darkBlue);
	keywordFormat.setFontWeight(QFont::Bold);
	commentFormat.setForeground(Qt::darkGray);
	mathFormat.setForeground(Qt::red);
	warningFormat.setForeground(Qt::red);
	warningFormat.setFontWeight(QFont::Bold);
}


void LaTeXHighlighter::highlightBlock(QString const & text)
{
	// $ $
	static const QRegExp exprMath("\\$[^\\$]*\\$");
	int index = text.indexOf(exprMath);
	while (index >= 0) {
		int length = exprMath.matchedLength();
		setFormat(index, length, mathFormat);
		index = text.indexOf(exprMath, index + length);
	}
	// [ ]
	static const QRegExp exprStartDispMath("(\\\\\\[|"
		"\\\\begin\\{equation\\**\\}|"
		"\\\\begin\\{eqnarray\\**\\}|"
		"\\\\begin\\{align(ed|at)*\\**\\}|"
		"\\\\begin\\{flalign\\**\\}|"
		"\\\\begin\\{gather\\**\\}|"
		"\\\\begin\\{multline\\**\\}|"
		"\\\\begin\\{array\\**\\}|"
		"\\\\begin\\{cases\\**\\}"
		")");
	static const QRegExp exprEndDispMath("(\\\\\\]|"
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
	// We try to avoid infinite loops...
	static size_t max_loop = 1000;
	for (size_t i = 0; i != max_loop; ++i) {
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
		if (startIndex == -1)
			break;
	}
	LASSERT(startIndex == -1, return);

	// \whatever
	static const QRegExp exprKeyword("\\\\[A-Za-z]+");
	index = text.indexOf(exprKeyword);
	for (size_t i = 0; i != max_loop; ++i) {
		int length = exprKeyword.matchedLength();
		setFormat(index, length, keywordFormat);
		index = text.indexOf(exprKeyword, index + length);
		if (index == -1)
			break;
	}
	LASSERT(index == -1, return);

	// %comment
	// Treat a line as a comment starting at a percent sign
	// * that is the first character in a line
	// * that is preceded by 
	// ** an even number of backslashes
	// ** any character other than a backslash
	QRegExp exprComment("(?:^|[^\\\\])(?:\\\\\\\\)*(%).*$"); 
	text.indexOf(exprComment);
	index = exprComment.pos(1);
	for (size_t i = 0; i != max_loop; ++i) {
		int const length = exprComment.matchedLength() 
				 - (index - exprComment.pos(0));
		setFormat(index, length, commentFormat);
		text.indexOf(exprComment, index + length);
		index = exprComment.pos(1);
		if (index == -1)
			break;
	}
	LASSERT(index == -1, return);

	// <LyX Warning: ...>
	QString lyxwarn = qt_("LyX Warning: ");
	QRegExp exprWarning("<" + lyxwarn + "[^<]*>");
	index = text.indexOf(exprWarning);
	for (size_t i = 0; i != max_loop; ++i) {
		int length = exprWarning.matchedLength();
		setFormat(index, length, warningFormat);
		index = text.indexOf(exprWarning, index + length);
		if (index == -1)
			break;
	}
	LASSERT(index == -1, return);
}

} // namespace frontend
} // namespace lyx
