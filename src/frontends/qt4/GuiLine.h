// -*- C++ -*-
/**
 * \file GuiLine.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUILINE_H
#define GUILINE_H

#include "GuiDialog.h"
#include "ui_LineUi.h"

#include "insets/InsetCommandParams.h"


namespace lyx {
namespace frontend {

class GuiLine : public GuiDialog, public Ui::LineUi
{
	Q_OBJECT

public:
	GuiLine(GuiView & lv);

private Q_SLOTS:
	void change_adaptor();

private:
	/// Apply changes
	void applyView();
	/// Update dialog before showing it
	bool initialiseParams(std::string const & data);
	///
	void paramsToDialog(InsetCommandParams const & icp);
	///
	void clearParams() { params_.clear(); }
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }
	///
	bool isValid() const;

	///
	InsetCommandParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUILINE_H
