// -*- C++ -*-
/**
 * \file QLog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QLOG_H
#define QLOG_H

#include "QDialogView.h"


#include "ui_LogUi.h"

#include <QDialog>
#include <QCloseEvent>
#include <QSyntaxHighlighter>

namespace lyx {
namespace frontend {

class QLog;

class QLogDialog : public QDialog, public Ui::QLogUi {
	Q_OBJECT
public:
	QLogDialog(QLog * form);
protected Q_SLOTS:
	virtual void updateClicked();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QLog * form_;
};


class ControlLog;
class LogHighlighter;

///
class QLog
	: public QController<ControlLog, QView<QLogDialog> >
{
public:
	///
	friend class QLogDialog;
	///
	QLog(Dialog &);
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
