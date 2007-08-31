// -*- C++ -*-
/**
 * \file GuiLog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUILOG_H
#define GUILOG_H

#include "GuiDialogView.h"


#include "ui_LogUi.h"

#include <QDialog>
#include <QCloseEvent>
#include <QSyntaxHighlighter>

namespace lyx {
namespace frontend {

class GuiLog;

class GuiLogDialog : public QDialog, public Ui::LogUi {
	Q_OBJECT
public:
	GuiLogDialog(GuiLog * form);
protected Q_SLOTS:
	virtual void updateClicked();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiLog * form_;
};


class ControlLog;
class LogHighlighter;

///
class GuiLog
	: public QController<ControlLog, GuiView<GuiLogDialog> >
{
public:
	///
	friend class GuiLogDialog;
	///
	GuiLog(Dialog &);
private:
	/// Apply changes
	virtual void apply() {}
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
	/// log syntax highlighter
	LogHighlighter * highlighter;
};


///
class LogHighlighter : public QSyntaxHighlighter
{
public:
	LogHighlighter(QTextDocument * parent);

protected:
	void highlightBlock(QString const & text);

private:
	QTextCharFormat infoFormat;
	QTextCharFormat warningFormat;
	QTextCharFormat errorFormat;
};


} // namespace frontend
} // namespace lyx

#endif // QLOG_H
