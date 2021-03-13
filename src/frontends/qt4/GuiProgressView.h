// -*- C++ -*-
/**
 * \file GuiProgressView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 * \author Pavel Sanda
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIPROGRESSVIEW_H
#define GUIPROGRESSVIEW_H

#include "ui_ProgressViewUi.h"

#include "DockView.h"


#include <string>


class QHideEvent;
class QShowEvent;


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
		Qt::WindowFlags flags = {});

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
	void restoreSession();
	void saveSession(QSettings & settings) const;
	///@}

private Q_SLOTS:
	void appendText(QString const & text);
	void appendLyXErrText(QString const & text);
	void clearText();
	void debugMessageActivated(QTreeWidgetItem *, int);
	void debugSelectionChanged();

private:
	ProgressViewWidget * widget_;
	/// did the last message contained eoln? (lyxerr X statusbar conflicts)
	bool eol_last_;

	void levelChanged();
	void showEvent(QShowEvent*);
	void hideEvent(QHideEvent*);
};


} // namespace frontend
} // namespace lyx

#endif

