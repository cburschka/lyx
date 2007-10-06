// -*- C++ -*-
/**
 * \file GuiLog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUILOG_H
#define GUILOG_H

#include "GuiDialog.h"
#include "ui_LogUi.h"

#include "support/FileName.h"


namespace lyx {
namespace frontend {

class LogHighlighter;

class GuiLog : public GuiDialog, public Ui::LogUi, public Controller
{
	Q_OBJECT

public:
	GuiLog(LyXView & lv);

private Q_SLOTS:
	void updateContents();

private:
	void closeEvent(QCloseEvent * e);
	/// parent controller
	Controller & controller() { return *this; }
	/// Apply changes
	void applyView() {}

	/// log syntax highlighter
	LogHighlighter * highlighter;

	/** \param data should contain "<logtype> <logfile>"
	 *  where <logtype> is one of "latex", "literate", "lyx2lyx", "vc".
	 */
	bool initialiseParams(std::string const & data);
	///
	void clearParams();
	///
	void dispatchParams() {}
	///
	bool isBufferDependent() const { return true; }

	/// The title displayed by the dialog reflects the \c LogType
	docstring title() const;
	/// put the log file into the ostream
	void getContents(std::ostream & ss) const;

private:
	/// Recognized log file-types
	enum LogType {
		LatexLog,
		LiterateLog,
		Lyx2lyxLog,
		VCLog
	};

	LogType type_;
	support::FileName logfile_;
};


} // namespace frontend
} // namespace lyx

#endif // GUILOG_H
