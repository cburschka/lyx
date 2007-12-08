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

#include <string>


namespace lyx {
namespace frontend {


class GuiProgress : 
	public DockView, 
	public lyx::support::ProgressInterface
{

public:
	GuiProgress(
		GuiView & parent, ///< the main window where to dock.
		Qt::DockWidgetArea area, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = 0);

	void appendMessage(QString const &);
	void clearMessages();

	/// Controller inherited method.
	///@{
	bool initialiseParams(std::string const & source) { return true; }
	void clearParams() {}
	void dispatchParams() {}
	bool isBufferDependent() const { return true; }
	bool canApply() const { return true; }
	bool canApplyToReadOnly() const { return true; }
	void updateView() {}
	///@}

private:
	QTextEdit text_edit;
};


} // namespace frontend
} // namespace lyx

#endif

