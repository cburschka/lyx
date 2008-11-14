// -*- C++ -*-
/**
 * \file GuiBranch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIBRANCH_H
#define GUIBRANCH_H

#include "GuiDialog.h"
#include "ui_BranchUi.h"
#include "insets/InsetBranch.h"


namespace lyx {
namespace frontend {

class GuiBranch : public GuiDialog, public Ui::BranchUi
{
	Q_OBJECT

public:
	GuiBranch(GuiView & lv);

private Q_SLOTS:
	void change_adaptor();

private:
	///  Apply changes
	void applyView();
	/// Update dialog before showing it
	void updateContents();
	///
	bool initialiseParams(std::string const & data);
	///
	void clearParams();
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }

	///
	InsetBranchParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIBRANCH_H
