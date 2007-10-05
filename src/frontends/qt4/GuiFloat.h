// -*- C++ -*-
/**
 * \file GuiFloat.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIFLOAT_H
#define GUIFLOAT_H

#include "GuiDialog.h"
#include "ui_FloatUi.h"
#include "insets/InsetFloat.h"


namespace lyx {
namespace frontend {

class GuiFloat : public GuiDialog, public Ui::FloatUi, public Controller
{
	Q_OBJECT

public:
	GuiFloat(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();

private:
	void closeEvent(QCloseEvent * e);
	/// parent controller
	Controller & controller() { return *this; }
	/// Apply changes
	void applyView();
	/// update
	void updateContents();
	///
	bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	void clearParams();
	/// clean-up on hide.
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }

private:
	///
	InsetFloatParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIFLOAT_H
