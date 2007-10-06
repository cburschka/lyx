// -*- C++ -*-
/**
 * \file GuiInclude.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIINCLUDE_H
#define GUIINCLUDE_H

#include "GuiDialog.h"
#include "ui_IncludeUi.h"

#include "insets/InsetCommandParams.h"

#include "support/docstring.h"


namespace lyx {
namespace frontend {

class GuiInclude : public GuiDialog, public Ui::IncludeUi, public Controller
{
	Q_OBJECT

public:
	GuiInclude(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();
	/// edit the child document, .lyx file will be opened in lyx
	/// other formats will be edited by external applications.
	void edit();
	/// browse for a file
	void browse();
	///
	void typeChanged(int v);
	/// AFAIK, QValidator only works for QLineEdit so
	/// I have to validate listingsED (QTextEdit) manually.
	/// This function displays a hint or error message returned by
	/// validate_listings_params
	void set_listings_msg();

private:
	void closeEvent(QCloseEvent * e);
	/// parent controller
	Controller & controller() { return *this; }
	///
	void updateLists();
	/// validate listings parameters and return an error message, if any
	docstring validate_listings_params();
	///
	void edit(std::string const & file);

	///
	bool isValid();
	/// Apply changes
	void applyView();
	/// update
	void updateContents();

	///
	enum Type {
		///
		INPUT,
		///
		VERBATIM,
		///
		INCLUDE,
		///
		LISTINGS,
	};

	///
	bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	void clearParams();
	/// clean-up on hide.
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }

	/// Browse for a file
	docstring browse(docstring const &, Type) const;

private:
	///
	InsetCommandParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIINCLUDE_H
