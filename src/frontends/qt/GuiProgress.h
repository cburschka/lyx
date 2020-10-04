// -*- C++ -*-
/**
 * \file GuiProgress.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 * \author Pavel Sanda
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIPROGRESS_H
#define GUIPROGRESS_H

#include "support/ProgressInterface.h"

#include <QTimer>

#include <sstream>


namespace lyx {
namespace frontend {


class GuiProgress :
		public QObject,
		public lyx::support::ProgressInterface
{
	Q_OBJECT

public:
	GuiProgress();
	~GuiProgress();

	void lyxerrConnect() override;
	void lyxerrDisconnect() override;
	void lyxerrFlush() override;

	int prompt(docstring const & title, docstring const & question,
		   int default_button, int cancel_button,
		   docstring const & b1, docstring const & b2) override;

	static QString currentTime();

Q_SIGNALS:

	// ProgressInterface
	void processStarted(QString const &) override;
	void processFinished(QString const &) override;
	void appendMessage(QString const &) override;
	void appendError(QString const &) override;
	void clearMessages() override;
	void appendLyXErrMessage(QString const & text);


	void clearMessageText();
	void updateStatusBarMessage(QString const &);
	void triggerFlush();

	// Alert interface
	void warning(QString const & title, QString const & message) override;
	void toggleWarning(QString const & title, QString const & msg, QString const & formatted) override;
	void error(QString const & title, QString const & message, QString const & details = QString()) override;
	void information(QString const & title, QString const & message) override;

private Q_SLOTS:
	void doProcessStarted(QString const &);
	void doProcessFinished(QString const &);
	void doAppendMessage(QString const &);
	void doAppendError(QString const &);
	void doClearMessages();

	void doWarning(QString const &, QString const &);
	void doToggleWarning(QString const & title, QString const & msg, QString const & formatted);
	void doError(QString const &, QString const &, QString const &);
	void doInformation(QString const &, QString const &);

	void updateWithLyXErr();
	void startFlushing();


private:
	void appendText(QString const &);
	std::ostringstream lyxerr_stream_;
	QTimer flushDelay_;
};



} // namespace frontend
} // namespace lyx

#endif

