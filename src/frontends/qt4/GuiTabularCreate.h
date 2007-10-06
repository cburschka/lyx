// -*- C++ -*-
/**
 * \file GuiTabularCreate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUITABULARCREATE_H
#define GUITABULARCREATE_H

#include "GuiDialog.h"
#include "ui_TabularCreateUi.h"

#include <utility>

namespace lyx {
namespace frontend {


class GuiTabularCreate
	: public GuiDialog, public Ui::TabularCreateUi, public Controller
{
	Q_OBJECT

public:
	GuiTabularCreate(LyXView & lv);

private Q_SLOTS:
	void columnsChanged(int);
	void rowsChanged(int);

	/// parent controller
	Controller & controller() { return *this; }

private:
	/// Apply changes
	void applyView();
	///
	bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	void clearParams();
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }
	///
	kb_action getLfun() const { return LFUN_TABULAR_INSERT; }

	///
	typedef std::pair<size_t, size_t> rowsCols;
	///
	rowsCols & params() { return params_; }

private:
	/// rows, cols params
	rowsCols params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUITABULARCREATE_H
