// -*- C++ -*-
/**
 * \file GuiProgress.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIPROGRESS_H
#define GUIPROGRESS_H

#include "support/ProgressInterface.h"

#include "DockView.h"

#include <QTextEdit>
#include <QSplashScreen>
#include <QTimer>

#include <string>


namespace lyx {
namespace frontend {


class GuiProgress : 
	public DockView, 
	public lyx::support::ProgressInterface
{

	Q_OBJECT
public:
	GuiProgress(
		GuiView & parent, ///< the main window where to dock.
		Qt::DockWidgetArea area, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = 0);

Q_SIGNALS:
	void processStarted(QString const &);
	void processFinished(QString const &);
	void appendMessage(QString const &);
	void appendError(QString const &);
	void clearMessages();

private Q_SLOTS:
	void doProcessStarted(QString const &);
	void doProcessFinished(QString const &);
	void doAppendMessage(QString const &);
	void doAppendError(QString const &);
	void doClearMessages();

public:
	/// Controller inherited method.
	///@{
	bool initialiseParams(std::string const &) { return true; }
	void clearParams() {}
	void dispatchParams() {}
	bool isBufferDependent() const { return false; }
	bool canApply() const { return true; }
	bool canApplyToReadOnly() const { return true; }
	void updateView() {}
	///@}

		
	void showEvent(QShowEvent*);
	void hideEvent(QHideEvent*);

private:
	QTextEdit text_edit;

	void appendText(QString const &);

};



} // namespace frontend
} // namespace lyx

#endif

