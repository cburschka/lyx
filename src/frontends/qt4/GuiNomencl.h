// -*- C++ -*-
/**
 * \file GuiNomencl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 * \author O. U. Baran
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUINOMENCL_H
#define GUINOMENCL_H

#include "GuiDialogView.h"
#include "ControlCommand.h"
#include "ui_NomenclUi.h"

#include <QDialog>


namespace lyx {
namespace frontend {

class GuiNomencl;

class GuiNomenclDialog : public QDialog, public Ui::NomenclUi {
	Q_OBJECT
public:
	GuiNomenclDialog(GuiNomencl * form);
	virtual void show();
protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void reject();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiNomencl * form_;
};


class GuiNomencl : public GuiView<GuiNomenclDialog>
{
public:
	friend class GuiNomenclDialog;

	GuiNomencl(Dialog &, docstring const & title);
	/// parent controller
	ControlCommand & controller()
	{ return static_cast<ControlCommand &>(this->getController()); }
	/// parent controller
	ControlCommand const & controller() const
	{ return static_cast<ControlCommand const &>(this->getController()); }
protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // GUINOMENCL_H
