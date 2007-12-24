// -*- C++ -*-
/**
 * \file LaTeXHighlighter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LATEXHIGHLIGHTER_H
#define LATEXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class QTextDocument;
class QString;

namespace lyx {
namespace frontend {

// used already twice...
class LaTeXHighlighter : public QSyntaxHighlighter
{
public:
	LaTeXHighlighter(QTextDocument * parent);

protected:
	void highlightBlock(QString const & text);

private:
	QTextCharFormat commentFormat;
	QTextCharFormat keywordFormat;
	QTextCharFormat mathFormat;
	QTextCharFormat warningFormat;
};

} // namespace frontend
} // namespace lyx

#endif // LATEXHIGHLIGHTER
