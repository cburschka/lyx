// -*- C++ -*-
/**
 * \file QListings.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QLISTINGS_H
#define QLISTINGS_H

#include "QDialogView.h"
#include "ui/ListingsUi.h"

#include <QDialog>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

class QListings;

class QListingsDialog : public QDialog, public Ui::QListingsUi {
	Q_OBJECT
public:
	QListingsDialog(QListings * form);
	/// get values from all the widgets and form a string
	std::string construct_params();
	/// validate listings parameters and return an error message, if any
	docstring validate_listings_params();
protected Q_SLOTS:
	virtual void change_adaptor();
	/// AFAIK, QValidator only works for QLineEdit so
	/// I have to validate listingsED (QTextEdit) manually.
	/// This function displays a hint or error message returned by
	/// validate_listings_params
	void set_listings_msg();
	/// turn off inline when float is clicked
	void on_floatCB_stateChanged(int state);
	/// turn off float when inline is clicked
	void on_inlineCB_stateChanged(int state);
	/// turn off numbering options when none is selected
	void on_numberSideCO_currentIndexChanged(int);
	/// show dialect when language is chosen
	void on_languageCO_currentIndexChanged(int);
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QListings * form_;
};


class ControlListings;

class QListings : public QController<ControlListings, QView<QListingsDialog> > {
public:
	friend class QListingsDialog;

	QListings(Dialog &);
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
protected:
	/// return false if validate_listings_params returns error
	virtual bool isValid();
};

} // namespace frontend
} // namespace lyx

#endif // QLISTINGS_H
