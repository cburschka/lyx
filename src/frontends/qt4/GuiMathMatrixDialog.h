// -*- C++ -*-
/**
 * \file GuiMathMatrixDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIMATHMATRIXDIALOG_H
#define GUIMATHMATRIXDIALOG_H

#include "GuiDialogView.h"
#include "ControlMath.h"
#include "ui_MathMatrixUi.h"

#include <QDialog>

namespace lyx {
namespace frontend {

class GuiMathMatrix;
class ControlMath;


class GuiMathMatrixDialog : public QDialog, public Ui::MathMatrixUi {
	Q_OBJECT
public:
	GuiMathMatrixDialog(GuiMathMatrix * form);
public Q_SLOTS:
	void slotOK();
	void slotClose();
protected Q_SLOTS:
	virtual void columnsChanged(int);
	virtual void rowsChanged(int);
	virtual void change_adaptor();
private:
	GuiMathMatrix * form_;
};


class GuiMathMatrix : public GuiView<GuiMathMatrixDialog> {
public:
	friend class GuiMathMatrixDialog;

	GuiMathMatrix(Dialog &);
	/// parent controller
	ControlMath & controller()
	{ return static_cast<ControlMath &>(this->getController()); }
	/// parent controller
	ControlMath const & controller() const
	{ return static_cast<ControlMath const &>(this->getController()); }
private:
	virtual void apply() {}
	virtual void update_contents() {}
	/// Build the dialog.
	virtual void build_dialog();
};


} // namespace frontend
} // namespace lyx

#endif // GUIMATHMATRIXDIALOG_H
