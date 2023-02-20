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


LaTeXHighlighter::LaTeXHighlighter(QTextDocument * parent, bool at_letter, bool keyval)
	: QSyntaxHighlighter(parent), at_letter_(at_letter), keyval_(keyval)
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
	keyFormat.setForeground(blend(Qt::darkRed, text_color));
	keyFormat.setFontWeight(QFont::Bold);
	valFormat.setForeground(blend(Qt::darkGreen, text_color));
}


void LaTeXHighlighter::highlightBlock(QString const & text)
{
	// keyval
	if (keyval_) {
		// Highlight key-val options. Used in some option widgets.
		static const QRegularExpression exprKeyvalkey("[^=,]+");
		static const QRegularExpression exprKeyvalval("[^,]+");
		QRegularExpressionMatch matchkey = exprKeyvalkey.match(text);
		int kvindex = matchkey.capturedStart(0);
		while (kvindex >= 0) {
			int length = matchkey.capturedLength(0);
			setFormat(kvindex, length, keyFormat);
			QRegularExpressionMatch matchval =
				exprKeyvalval.match(text, kvindex + length);
			int kvvindex = matchval.capturedStart(0);
			if (kvvindex > 0) {
				length += matchval.capturedLength(0);
				setFormat(kvvindex, length, valFormat);
			}
			matchkey = exprKeyvalkey.match(text, kvindex + length);
			kvindex = matchkey.capturedStart(0);
		}
	}
	// $ $
	static const QRegularExpression exprMath("\\$[^\\$]*\\$");
	QRegularExpressionMatch match = exprMath.match(text);
	int index = match.capturedStart(0);
	while (index >= 0) {
		int length = match.capturedLength(0);
		setFormat(index, length, mathFormat);
		match = exprMath.match(text, index + length);
		index = match.capturedStart(0);
	}
	// [ ]
	static const QRegularExpression exprStartDispMath("(\\\\\\[|"
		"\\\\begin\\{equation\\**\\}|"
		"\\\\begin\\{eqnarray\\**\\}|"
		"\\\\begin\\{align(ed|at)*\\**\\}|"
		"\\\\begin\\{flalign\\**\\}|"
		"\\\\begin\\{gather\\**\\}|"
		"\\\\begin\\{multline\\**\\}|"
		"\\\\begin\\{array\\**\\}|"
		"\\\\begin\\{cases\\**\\}"
		")");
	static const QRegularExpression exprEndDispMath("(\\\\\\]|"
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
	if (previousBlockState() != 1) {
		match = exprStartDispMath.match(text);
		startIndex = match.capturedStart(0);
	}
	while (startIndex >= 0) {
		match = exprEndDispMath.match(text, startIndex);
		int endIndex = match.capturedStart(0);
		int length;
		if (endIndex == -1) {
			setCurrentBlockState(1);
			length = text.length() - startIndex;
		} else {
			length = endIndex - startIndex + match.capturedLength(0);
		}
		setFormat(startIndex, length, mathFormat);
		match = exprStartDispMath.match(text, startIndex + length);
		startIndex = match.capturedStart(0);
	}
	// \whatever
	static const QRegularExpression exprKeywordAtOther("\\\\[A-Za-z]+");
	// \wh@tever
	static const QRegularExpression exprKeywordAtLetter("\\\\[A-Za-z@]+");
	QRegularExpression const & exprKeyword = at_letter_
			? exprKeywordAtLetter : exprKeywordAtOther;
	match = exprKeyword.match(text);
	index = match.capturedStart(0);
	while (index >= 0) {
		int length = match.capturedLength(0);
		setFormat(index, length, keywordFormat);
		match = exprKeyword.match(text, index + length);
		index = match.capturedStart(0);
	}
	// %comment
	// Treat a line as a comment starting at a percent sign
	// * that is the first character in a line
	// * that is preceded by
	// ** an even number of backslashes
	// ** any character other than a backslash
	QRegularExpression exprComment("(?:^|[^\\\\])(?:\\\\\\\\)*(%).*$");
	match = exprComment.match(text);
	index = match.capturedStart(1);
	while (index >= 0) {
		int const length = match.capturedLength(0)
				 - (index - match.capturedStart(0));
		setFormat(index, length, commentFormat);
		match = exprComment.match(text, index + length);
		index = match.capturedStart(1);
	}
	// <LyX Warning: ...>
	QString lyxwarn = qt_("LyX Warning: ");
	QRegularExpression exprWarning("<" + lyxwarn + "[^<]*>");
	match = exprWarning.match(text);
	index = match.capturedStart(0);
	while (index >= 0) {
		int length = match.capturedLength(0);
		setFormat(index, length, warningFormat);
		match = exprWarning.match(text, index + length);
		index = match.capturedStart(0);
	}
}

} // namespace frontend
} // namespace lyx
