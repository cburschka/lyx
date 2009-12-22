// -*- C++ -*-
/**
 * \file GuiProgressView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIPROGRESSVIEW_H
#define GUIPROGRESSVIEW_H


#include "DockView.h"

#include "GuiProgress.h"

#include <QTextEdit>

#include <string>


namespace lyx {
namespace frontend {


class GuiProgressView : public DockView
{
	Q_OBJECT

public:
	GuiProgressView(
		GuiView & parent, ///< the main window where to dock.
		Qt::DockWidgetArea area, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = 0);

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

private Q_SLOTS:
	void appendText(QString const & text);
	void clearText();

private:
	QTextEdit text_edit;
};


} // namespace frontend
} // namespace lyx

#endif

