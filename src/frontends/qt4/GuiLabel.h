// -*- C++ -*-
/**
 * \file GuiLabel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUILABEL_H
#define GUILABEL_H

#include "GuiDialog.h"
#include "ui_LabelUi.h"

#include "insets/InsetCommandParams.h"

namespace lyx {
namespace frontend {

class GuiLabel : public GuiDialog, public Ui::LabelUi
{
	Q_OBJECT

public:
	GuiLabel(GuiView & lv);

private Q_SLOTS:
	void change_adaptor();
	void reject();

private:
	///
	bool isValid();
	/// Apply changes
	void applyView();
	/// update
	void updateContents();
	///
	bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	void clearParams() { params_.clear(); }
	/// clean-up on hide.
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }

private:
	///
	InsetCommandParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUILABEL_H
