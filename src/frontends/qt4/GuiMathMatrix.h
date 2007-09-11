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

#include "GuiDialog.h"
#include "ControlMath.h"
#include "ui_MathMatrixUi.h"

#include <QDialog>

namespace lyx {
namespace frontend {

class GuiMathMatrixDialog : public GuiDialog, public Ui::MathMatrixUi
{
	Q_OBJECT

public:
	GuiMathMatrixDialog(LyXView & lv);

public Q_SLOTS:
	void slotOK();
	void slotClose();
	void columnsChanged(int);
	void rowsChanged(int);
	void change_adaptor();

	/// parent controller
	ControlMath & controller();
};

} // namespace frontend
} // namespace lyx

#endif // GUIMATHMATRIXDIALOG_H
