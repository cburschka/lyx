// -*- C++ -*-
/**
 * \file GuiViewSource.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Bo Peng
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIVIEWSOURCE_H
#define GUIVIEWSOURCE_H

#include "GuiDialog.h"
#include "ControlViewSource.h"
#include "Application.h"
#include "ui_ViewSourceUi.h"

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
};


class GuiViewSourceDialog : public GuiDialog, public Ui::ViewSourceUi
{
	Q_OBJECT

public:
	GuiViewSourceDialog(LyXView & lv);

public Q_SLOTS:
	// update content
	void updateView();
	///
	QTextDocument * document() { return document_; }
	///
	void update(bool full_source);

private:
	///
	ControlViewSource & controller() const;	
	///
	QTextDocument * document_;
	/// LaTeX syntax highlighter
	LaTeXHighlighter * highlighter_;
};


} // namespace frontend
} // namespace lyx

#endif // GUIVIEWSOURCE_H
