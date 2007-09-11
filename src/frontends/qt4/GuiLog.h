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

#include "GuiDialog.h"
#include "ControlLog.h"
#include "ui_LogUi.h"

namespace lyx {
namespace frontend {

class LogHighlighter;

class GuiLogDialog : public GuiDialog, public Ui::LogUi
{
	Q_OBJECT
public:
	GuiLogDialog(LyXView & lv);
private Q_SLOTS:
	void updateClicked();
private:
	void closeEvent(QCloseEvent * e);
	/// parent controller
	ControlLog & controller();
	/// Apply changes
	void applyView() {}
	/// update
	void updateContents();

	/// log syntax highlighter
	LogHighlighter * highlighter;
};


} // namespace frontend
} // namespace lyx

#endif // QLOG_H
