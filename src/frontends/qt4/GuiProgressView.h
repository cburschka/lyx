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

#include "ui_ProgressViewUi.h"

#include "DockView.h"

#include "GuiProgress.h"

#include <string>

#include <QDockWidget>
#include "qt_helpers.h" 

namespace lyx {
namespace frontend {

class ProgressViewWidget : public QWidget, public Ui::ProgressViewUi
{
	Q_OBJECT

public:
	ProgressViewWidget();
private:
	
};

class GuiProgressView : public DockView
{
	Q_OBJECT

public:
	GuiProgressView(
		GuiView & parent, ///< the main window where to dock.
		Qt::DockWidgetArea area, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = 0);

	~GuiProgressView();
	/// Controller inherited method.
	///@{
	bool initialiseParams(std::string const &) { return true; }
	void clearParams() {}
	void dispatchParams() {}
	bool isBufferDependent() const { return false; }
	bool canApply() const { return true; }
	bool canApplyToReadOnly() const { return true; }
	void updateView() {}
	bool wantInitialFocus() const { return false; }
	///@}

private Q_SLOTS:
	void appendText(QString const & text);
	void clearText();

private:
	ProgressViewWidget * widget_;
};


} // namespace frontend
} // namespace lyx

#endif

