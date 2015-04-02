// -*- C++ -*-
/**
 * \file GuiPrintNomencl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIPRINTNOMENCL_H
#define GUIPRINTNOMENCL_H

#include "InsetParamsWidget.h"
#include "ui_PrintNomenclUi.h"

namespace lyx {

class InsetCommandParams;

namespace frontend {

class GuiPrintNomencl : public InsetParamsWidget, public Ui::PrintNomenclUi
{
	Q_OBJECT

public:
	GuiPrintNomencl(QWidget * parent = 0);

private Q_SLOTS:
	void on_setWidthCO_activated(int);

private:
	/// \name InsetParamsWidget inherited methods
	//@{
	InsetCode insetCode() const { return NOMENCL_PRINT_CODE; }
	FuncCode creationCode() const { return LFUN_INSET_INSERT; }
	void paramsToDialog(Inset const *);
	void paramsToDialog(InsetCommandParams const &);
	docstring dialogToParams() const;
	bool checkWidgets(bool readonly) const;
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // GUIPRINTNOMENCL_H
