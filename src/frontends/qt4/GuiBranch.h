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

#ifndef GUIBRANCH_H
#define GUIBRANCH_H

#include "GuiDialogView.h"
#include "ControlBranch.h"
#include "ui_BranchUi.h"

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


/** This class provides a QT implementation of the Branch Dialog.
 */
class GuiBranch : public GuiView<GuiBranchDialog> 
{
public:
	/// Constructor
	GuiBranch(GuiDialog &);
	/// parent controller
	ControlBranch & controller()
	{ return static_cast<ControlBranch &>(this->getController()); }
	/// parent controller
	ControlBranch const & controller() const
	{ return static_cast<ControlBranch const &>(this->getController()); }
private:
	friend class GuiBranchDialog;

	///  Apply changes
	virtual void apply();
	/// Build the dialog
	virtual void build_dialog();
	/// Update dialog before showing it
	virtual void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // GUIBRANCH_H
