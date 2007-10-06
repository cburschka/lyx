// -*- C++ -*-
/**
 * \file GuiErrorList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIERRORLIST_H
#define GUIERRORLIST_H

#include "GuiDialog.h"
#include "ErrorList.h"
#include "ui_ErrorListUi.h"

class QListWidgetItem;

namespace lyx {
namespace frontend {

class GuiErrorList : public GuiDialog, public Ui::ErrorListUi, public Controller
{
	Q_OBJECT

public:
	GuiErrorList(LyXView & lv);

public Q_SLOTS:
	/// select an entry
	void select(QListWidgetItem *);

private:
	///
	void closeEvent(QCloseEvent *);
	///
	void showEvent(QShowEvent *);
	/// parent controller
	Controller & controller() { return *this; }
	/// update contents
	void updateContents();
	///
	bool isBufferDependent() const { return true; }
	///
	bool initialiseParams(std::string const & data);
	///
	void clearParams() {}
	///
	void dispatchParams() {}

	/// goto this error in the parent bv
	void goTo(int item);
	///
	ErrorList const & errorList() const;
private:
	///
	std::string error_type_;
	/// the parent document name
	docstring name_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIERRORLIST_H
