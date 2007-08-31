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

#ifndef QTABULARCREATE_H
#define QTABULARCREATE_H

#include "GuiDialogView.h"
#include "ui_TabularCreateUi.h"

#include <QDialog>

namespace lyx {
namespace frontend {

class GuiTabularCreate;

class GuiTabularCreateDialog : public QDialog, public Ui::TabularCreateUi {
	Q_OBJECT
public:
	GuiTabularCreateDialog(GuiTabularCreate * form);
protected Q_SLOTS:
	virtual void columnsChanged(int);
	virtual void rowsChanged(int);
private:
	GuiTabularCreate * form_;
};


class ControlTabularCreate;

class GuiTabularCreate
	: public QController<ControlTabularCreate, GuiView<GuiTabularCreateDialog> >
{
public:
	///
	friend class GuiTabularCreateDialog;
	///
	GuiTabularCreate(Dialog &);
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents() {}
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // QTABULARCREATE_H
