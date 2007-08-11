// -*- C++ -*-
/**
 * \file QBox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 * \ author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBOX_H
#define QBOX_H

#include "QDialogView.h"

#include "ui_BoxUi.h"

#include <QCloseEvent>
#include <QDialog>

#include <vector>


namespace lyx {
namespace frontend {

class ControlBox;
class QBox;

class QBoxDialog : public QDialog, public Ui::QBoxUi {
	Q_OBJECT
public:
	QBoxDialog(QBox * form);
protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void innerBoxChanged(const QString &);
	virtual void typeChanged(int);
	virtual void restoreClicked();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QBox * form_;
};


///
class QBox
	: public QController<ControlBox, QView<QBoxDialog> >
{
public:
	///
	friend class QBoxDialog;
	///
	QBox(Dialog &);
	/// add and remove special lengths
	void setSpecial(bool ibox);
	/// only show valid inner box items
	void setInnerType(bool frameless, int i);
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
	///
	std::vector<std::string> ids_;
	///
	std::vector<docstring> gui_names_;
	///
	std::vector<std::string> ids_spec_;
	///
	std::vector<docstring> gui_names_spec_;
};

} // namespace frontend
} // namespace lyx

#endif // QBOX_H
