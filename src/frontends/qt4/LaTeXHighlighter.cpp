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

#include <QString>
#include <QTextDocument>

namespace lyx {
namespace frontend {


LaTeXHighlighter::LaTeXHighlighter(QTextDocument * parent, bool at_letter)
	: QSyntaxHighlighter(parent), at_letter_(at_letter)
{
	auto blend = [](QColor color1, QColor color2) {
		int r = 0.5 * (color1.red() + color2.red());
		int g = 0.5 * (color1.green() + color2.green());
		int b = 0.5 * (color1.blue() + color2.blue());
		return QColor(r, g, b);
	};
	QPalette palette = QPalette();
	QColor text_color = palette.color(QPalette::Active, QPalette::Text);
	keywordFormat.setForeground(blend(Qt::blue, text_color));
	keywordFormat.setFontWeight(QFont::Bold);
	commentFormat.setForeground(palette.color(QPalette::Disabled,
	                                          QPalette::Text));
	mathFormat.setForeground(blend(Qt::red, text_color));
	warningFormat.setForeground(Qt::red);
	warningFormat.setFontWeight(QFont::Bold);
}


void LaTeXHighlighter::highlightBlock(QString const & text)
{
	// $ $
	static const QRegExp exprMath("\\$[^\\$]*\\$");
	int index = exprMath.indexIn(text);
	while (index >= 0) {
		int length = exprMath.matchedLength();
		setFormat(index, length, mathFormat);
		index = exprMath.indexIn(text, index + length);
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
		startIndex = exprStartDispMath.indexIn(text);
	while (startIndex >= 0) {
		int endIndex = exprEndDispMath.indexIn(text, startIndex);
		int length;
		if (endIndex == -1) {
			setCurrentBlockState(1);
			length = text.length() - startIndex;
		} else {
			length = endIndex - startIndex + exprEndDispMath.matchedLength();
		}
		setFormat(startIndex, length, mathFormat);
		startIndex = exprStartDispMath.indexIn(text, startIndex + length);
	}
	// \whatever
	static const QRegExp exprKeywordAtOther("\\\\[A-Za-z]+");
	// \wh@tever
	static const QRegExp exprKeywordAtLetter("\\\\[A-Za-z@]+");
	QRegExp const & exprKeyword = at_letter_ ? exprKeywordAtLetter
	                                         : exprKeywordAtOther;
	index = exprKeyword.indexIn(text);
	while (index >= 0) {
		int length = exprKeyword.matchedLength();
		setFormat(index, length, keywordFormat);
		index = exprKeyword.indexIn(text, index + length);
	}
	// %comment
	// Treat a line as a comment starting at a percent sign
	// * that is the first character in a line
	// * that is preceded by 
	// ** an even number of backslashes
	// ** any character other than a backslash
	QRegExp exprComment("(?:^|[^\\\\])(?:\\\\\\\\)*(%).*$"); 
	exprComment.indexIn(text);
	index = exprComment.pos(1);
	while (index >= 0) {
		int const length = exprComment.matchedLength() 
				 - (index - exprComment.pos(0));
		setFormat(index, length, commentFormat);
		exprComment.indexIn(text, index + length);
		index = exprComment.pos(1);
	}
	// <LyX Warning: ...>
	QString lyxwarn = qt_("LyX Warning: ");
	QRegExp exprWarning("<" + lyxwarn + "[^<]*>");
	index = exprWarning.indexIn(text);
	while (index >= 0) {
		int length = exprWarning.matchedLength();
		setFormat(index, length, warningFormat);
		index = exprWarning.indexIn(text, index + length);
	}
}

} // namespace frontend
} // namespace lyx
