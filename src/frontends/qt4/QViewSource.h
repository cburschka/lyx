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
#include "frontends/Application.h"
#include "ui/ViewSourceUi.h"

#include <QWidget>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class QTextDocument;

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



class QViewSource;

class QViewSourceDialog : public QWidget, public Ui::QViewSourceUi {
	Q_OBJECT
public:
	QViewSourceDialog(QViewSource * form);

public Q_SLOTS:
	// update content
	void update();

private:
	QViewSource * form_;
};


///
class QViewSource : public QObject, public ControlViewSource {
public:
	///
	QViewSource(Dialog &);
	///
	QTextDocument * document() { return document_; }
	///
	void update(bool full_source);

private:
	///
	QTextDocument * document_;
	/// LaTeX syntax highlighter
	LaTeXHighlighter * highlighter_;
};


} // namespace frontend
} // namespace lyx

#endif // QVIEWSOURCE_H
