// -*- C++ -*-
/**
 * \file GuiPrintindex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIPRINTINDEX_H
#define GUIPRINTINDEX_H

#include "GuiDialog.h"
#include "ui_PrintindexUi.h"
#include "insets/InsetCommandParams.h"


namespace lyx {
namespace frontend {

class GuiPrintindex : public GuiDialog, public Ui::PrintindexUi
{
	Q_OBJECT

public:
	GuiPrintindex(GuiView & lv);

private Q_SLOTS:
	void change_adaptor();

private:
	///  Apply changes
	void applyView();
	/// Update dialog before showing it
	void updateContents();
	///
	void paramsToDialog(InsetCommandParams const & icp);
	///
	bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	void clearParams() { params_.clear(); }
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }

	///
	InsetCommandParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIPRINTINDEX_H
