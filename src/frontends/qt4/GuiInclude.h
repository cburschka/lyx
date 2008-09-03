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


namespace lyx {
namespace frontend {

class GuiInclude : public GuiDialog, public Ui::IncludeUi
{
	Q_OBJECT

public:
	GuiInclude(GuiView & lv);

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
	///
	bool initialiseParams(std::string const & data);
	///
	void paramsToDialog(InsetCommandParams const & params_);
	/// clean-up on hide.
	void clearParams() { params_.clear(); }
	/// clean-up on hide.
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }

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
	void updateLists();
	/// validate listings parameters and return an error message, if any
	docstring validate_listings_params();
	///
	bool isValid();
	/// Apply changes
	void applyView();
	/// update
	void updateContents() {}
	/// Browse for a file
	QString browse(QString const &, Type) const;

private:
	///
	InsetCommandParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIINCLUDE_H
