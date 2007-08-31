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

#include "GuiDialogView.h"
#include "ControlErrorList.h"
#include "ui_ErrorListUi.h"

#include <QDialog>

class QListWidgetItem;

namespace lyx {
namespace frontend {

class GuiErrorList;

class GuiErrorListDialog : public QDialog, public Ui::ErrorListUi {
	Q_OBJECT
public:
	GuiErrorListDialog(GuiErrorList * form);

public Q_SLOTS:
	void select_adaptor(QListWidgetItem *);
protected:
	void closeEvent(QCloseEvent *);
	void showEvent(QShowEvent *);
private:
	GuiErrorList * form_;
};


class GuiErrorList : public GuiView<GuiErrorListDialog>
{
public:
	friend class GuiErrorListDialog;

	GuiErrorList(Dialog &);
	/// parent controller
	ControlErrorList & controller()
	{ return static_cast<ControlErrorList &>(this->getController()); }
	/// parent controller
	ControlErrorList const & controller() const
	{ return static_cast<ControlErrorList const &>(this->getController()); }
private:
	/// select an entry
	void select(QListWidgetItem *);
	/// required apply
	virtual void apply() {}
	/// build dialog
	virtual void build_dialog();
	/// update contents
	virtual void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // GUIERRORLIST_H
