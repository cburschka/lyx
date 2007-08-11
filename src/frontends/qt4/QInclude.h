// -*- C++ -*-
/**
 * \file QInclude.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QINCLUDE_H
#define QINCLUDE_H

#include "QDialogView.h"

#include "ui_IncludeUi.h"

#include <QDialog>

namespace lyx {
namespace frontend {

class QInclude;

class QIncludeDialog : public QDialog, public Ui::QIncludeUi {
	Q_OBJECT
public:
	QIncludeDialog(QInclude * form);

	void updateLists();

	virtual void show();
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
	QInclude * form_;
};


class ControlInclude;

///
class QInclude : public QController<ControlInclude, QView<QIncludeDialog> >
{
public:
	///
	friend class QIncludeDialog;
	///
	QInclude(Dialog &);
protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void apply();
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

#endif // QINCLUDE_H
