// -*- C++ -*-
/**
 * \file GuiMathMatrix.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIMATHMATRIX_H
#define GUIMATHMATRIX_H

#include "GuiDialog.h"

#include "ui_MathMatrixUi.h"

namespace lyx {
namespace frontend {

class GuiMathMatrix : public GuiDialog, public Ui::MathMatrixUi
{
	Q_OBJECT

public:
	GuiMathMatrix(GuiView & lv);

	bool initialiseParams(std::string const &) { return true; }
	void clearParams() {}
	void dispatchParams() {}
	bool isBufferDependent() const { return true; }

public Q_SLOTS:
	void slotOK();
	void slotClose();
	void columnsChanged(int);
	void rowsChanged(int);
	void change_adaptor();
};

} // namespace frontend
} // namespace lyx

#endif // GUIMATHMATRIXDIALOG_H
