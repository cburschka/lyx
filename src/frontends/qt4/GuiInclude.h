// -*- C++ -*-
/**
 * \file GuiInclude.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIINCLUDE_H
#define GUIINCLUDE_H

#include "GuiDialog.h"
#include "ControlInclude.h"
#include "ui_IncludeUi.h"

namespace lyx {
namespace frontend {

class GuiIncludeDialog : public GuiDialog, public Ui::IncludeUi
{
	Q_OBJECT

public:
	GuiIncludeDialog(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();
	void editClicked();
	void browseClicked();
	void typeChanged(int v);
	/// AFAIK, QValidator only works for QLineEdit so
	/// I have to validate listingsED (QTextEdit) manually.
	/// This function displays a hint or error message returned by
	/// validate_listings_params
	void set_listings_msg();

private:
	void closeEvent(QCloseEvent * e);
	/// parent controller
	ControlInclude & controller();
	///
	void updateLists();
	/// validate listings parameters and return an error message, if any
	docstring validate_listings_params();

	///
	bool isValid();
	/// Apply changes
	void applyView();
	/// update
	void updateContents();
	/// edit the child document, .lyx file will be opened in lyx
	/// other formats will be edited by external applications.
	void edit();
	/// browse for a file
	void browse();
};

} // namespace frontend
} // namespace lyx

#endif // GUIINCLUDE_H
