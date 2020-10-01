// -*- C++ -*-
/**
 * \file GuiIndex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIINDEX_H
#define GUIINDEX_H

#include "GuiDialog.h"
#include "ui_IndexUi.h"
#include "insets/InsetIndex.h"


namespace lyx {
namespace frontend {

class GuiIndex : public GuiDialog, public Ui::IndexUi
{
	Q_OBJECT

public:
	GuiIndex(GuiView & lv);

private Q_SLOTS:
	void change_adaptor();

private:
	///  Apply changes
	void applyView() override;
	/// Update dialog before showing it
	void updateContents() override;
	///
	bool initialiseParams(std::string const & data) override;
	///
	void clearParams() override;
	///
	void dispatchParams() override;
	///
	bool isBufferDependent() const override { return true; }

	///
	InsetIndexParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIINDEX_H
