// -*- C++ -*-
/**
 * \file GuiLine.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUILINE_H
#define GUILINE_H

#include "InsetParamsWidget.h"
#include "ui_LineUi.h"

namespace lyx {
namespace frontend {

class GuiLine : public InsetParamsWidget, public Ui::LineUi
{
	Q_OBJECT

public:
	GuiLine(QWidget * parent = 0);

private:
	/// \name InsetParamsWidget inherited methods
	//@{
	InsetCode insetCode() const { return LINE_CODE; }
	FuncCode creationCode() const { return LFUN_INSET_INSERT; }
	QString dialogTitle() const { return qt_("Line Settings"); }
	void paramsToDialog(Inset const *);
	docstring dialogToParams() const;
	bool checkWidgets(bool readonly) const;
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // GUILINE_H
