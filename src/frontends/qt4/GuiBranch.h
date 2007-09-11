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

#include "GuiDialog.h"
#include "ControlBranch.h"
#include "ui_BranchUi.h"

namespace lyx {
namespace frontend {

class GuiBranchDialog : public GuiDialog, public Ui::BranchUi
{
	Q_OBJECT

public:
	GuiBranchDialog(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();

private:
	///
	void closeEvent(QCloseEvent * e);
	/// parent controller
	ControlBranch & controller();
	///  Apply changes
	void applyView();
	/// Update dialog before showing it
	void updateContents();
};


} // namespace frontend
} // namespace lyx

#endif // GUIBRANCH_H
