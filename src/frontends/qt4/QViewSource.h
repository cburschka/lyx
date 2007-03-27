// -*- C++ -*-
/**
 * \file QViewSource.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Bo Peng
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QVIEWSOURCE_H
#define QVIEWSOURCE_H

#include "frontends/controllers/ControlViewSource.h"

#include <QObject>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextDocument>

namespace lyx {
namespace frontend {

/// LaTeX syntax highlighting.
/// \todo FIXME: extract the latexHighlighter class into its 
/// own .[Ch] files.
class latexHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT	
public:
	latexHighlighter(QTextDocument * parent);

protected:
	void highlightBlock(QString const & text);

private:
	QTextCharFormat commentFormat;
	QTextCharFormat keywordFormat;
	QTextCharFormat mathFormat;
};

///
class QViewSource: public QObject, public ControlViewSource
{
	Q_OBJECT
public:
	QViewSource(Dialog &);
	virtual ~QViewSource() {}

	QTextDocument * document() { return document_; }

	void update(bool full_source);

private:
	///
	QTextDocument * document_;

	/// latex syntax highlighter
	latexHighlighter * highlighter_;
};


} // namespace frontend
} // namespace lyx

#endif // QVIEWSOURCE_H
