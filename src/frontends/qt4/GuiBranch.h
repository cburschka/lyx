// -*- C++ -*-
/**
 * \file GuiBranch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBRANCH_H
#define QBRANCH_H

#include "GuiDialogView.h"
#include "ui_BranchUi.h"

#include <QCloseEvent>
#include <QDialog>

namespace lyx {
namespace frontend {

class GuiBranch;

class GuiBranchDialog : public QDialog, public Ui::BranchUi {
	Q_OBJECT
public:
	GuiBranchDialog(GuiBranch * form);
protected Q_SLOTS:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiBranch * form_;
};

class ControlBranch;

/** This class provides a QT implementation of the Branch Dialog.
 */
class GuiBranch : public QController<ControlBranch, GuiView<GuiBranchDialog> >
{
public:
	friend class GuiBranchDialog;

	/// Constructor
	GuiBranch(Dialog &);
private:
	///  Apply changes
	virtual void apply();
	/// Build the dialog
	virtual void build_dialog();
	/// Update dialog before showing it
	virtual void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // QBRANCH_H
