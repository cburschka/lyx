// -*- C++ -*-
/**
 * \file GuiWrap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIWRAP_H
#define GUIWRAP_H

#include "GuiDialog.h"
#include "ui_WrapUi.h"
#include "insets/InsetWrap.h"

namespace lyx {
namespace frontend {

class GuiWrap : public GuiDialog, public Ui::WrapUi, public Controller
{
	Q_OBJECT

public:
	GuiWrap(LyXView & lv);

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

	///
	InsetWrapParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIWRAP_H
