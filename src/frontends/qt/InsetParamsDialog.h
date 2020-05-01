// -*- C++ -*-
/**
 * \file InsetParamsDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_PARAMS_DIALOG_H
#define INSET_PARAMS_DIALOG_H

#include "DialogView.h"
#include "ui_InsetParamsUi.h"

#include <QPushButton>

namespace lyx {

class Inset;

namespace frontend {

class InsetParamsWidget;

/// An InsetParamsDialog wraps an InsetParamsWidget, which is what
/// will contain all the specific dialog parts for a given inset.
/// This class manages the OK, etc, buttons and immediate apply
/// checkbox, etc.
class InsetParamsDialog : public DialogView, public Ui::InsetParamsUi
{
	Q_OBJECT
public:
	InsetParamsDialog(GuiView & lv, InsetParamsWidget * widget);
	~InsetParamsDialog();

	///
	void setInsetParamsWidget(InsetParamsWidget * widget);

protected Q_SLOTS:
	void onWidget_changed();
	void applyView();
	void resetDialog();
	void on_buttonBox_clicked(QAbstractButton *);
	void on_immediateApplyCB_stateChanged(int state);
	void on_synchronizedCB_stateChanged(int state);

private:
	/// \name DialogView inherited methods
	//@{
	void updateView();
	void dispatchParams() {}
	bool isBufferDependent() const { return true; }
	bool canApply() const { return true; }
	bool initialiseParams(std::string const &);
	//@}
	///
	void newInset();
	///
	bool newInsetAllowed() const;
	///
	void updateView(bool update_widget);
	///
	docstring checkWidgets(bool immediate);
	///
	QPushButton * newPB;
	/// pimpl
	struct Private;
	Private * d;
};

} // namespace frontend
} // namespace lyx

#endif // INSET_PARAMS_DIALOG_H
