/**
 * \file LaTeXHighlighter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include "LaTeXHighlighter.h"

#include <QString>
#include <QTextDocument>

namespace lyx {
namespace frontend {

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
	static const QRegExp exprKeyword("\\\\[A-Za-z]+");
	index = text.indexOf(exprKeyword);
	while (index >= 0) {
		int length = exprKeyword.matchedLength();
		setFormat(index, length, keywordFormat);
		index = text.indexOf(exprKeyword, index + length);
	}
	// comment
	static const QRegExp exprComment("(^|[^\\\\])%.*$");
	index = text.indexOf(exprComment);
	while (index >= 0) {
		int const length = exprComment.matchedLength();
		setFormat(index, length, commentFormat);
		index = text.indexOf(exprComment, index + length);
	}
}

} // namespace frontend
} // namespace lyx
