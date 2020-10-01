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


class GuiTabularCreate : public GuiDialog, public Ui::TabularCreateUi
{
	Q_OBJECT

public:
	GuiTabularCreate(GuiView & lv);

private Q_SLOTS:
	void columnsChanged(int);
	void rowsChanged(int);
	void on_styleCO_activated(int);

private:
	/// Apply changes
	void applyView() override;
	///
	bool initialiseParams(std::string const & data) override;
	/// clean-up on hide.
	void clearParams() override;
	///
	void dispatchParams() override;
	///
	bool isBufferDependent() const override { return true; }
	///
	FuncCode getLfun() const override;

	///
	typedef std::pair<size_t, size_t> rowsCols;
	///
	rowsCols & params() { return params_; }
	///
	void getFiles();

private:
	/// rows, cols params
	rowsCols params_;
	///
	QString style_;
};

} // namespace frontend
} // namespace lyx

#endif // GUITABULARCREATE_H
