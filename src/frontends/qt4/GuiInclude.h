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

#include "GuiDialogView.h"
#include "ControlInclude.h"
#include "ui_IncludeUi.h"

#include <QDialog>

namespace lyx {
namespace frontend {

class GuiInclude;

class GuiIncludeDialog : public QDialog, public Ui::IncludeUi {
	Q_OBJECT
public:
	GuiIncludeDialog(GuiInclude * form);

	void updateLists();

	virtual void showView();
	/// validate listings parameters and return an error message, if any
	docstring validate_listings_params();
protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void editClicked();
	virtual void browseClicked();
	virtual void typeChanged(int v);
	/// AFAIK, QValidator only works for QLineEdit so
	/// I have to validate listingsED (QTextEdit) manually.
	/// This function displays a hint or error message returned by
	/// validate_listings_params
	void set_listings_msg();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiInclude * form_;
};


class GuiInclude : public GuiView<GuiIncludeDialog>
{
public:
	///
	friend class GuiIncludeDialog;
	///
	GuiInclude(GuiDialog &);
	/// parent controller
	ControlInclude & controller()
	{ return static_cast<ControlInclude &>(this->getController()); }
	/// parent controller
	ControlInclude const & controller() const
	{ return static_cast<ControlInclude const &>(this->getController()); }
protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void applyView();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	/// edit the child document, .lyx file will be opened in lyx
	/// other formats will be edited by external applications.
	void edit();

	/// browse for a file
	void browse();
};

} // namespace frontend
} // namespace lyx

#endif // GUIINCLUDE_H
