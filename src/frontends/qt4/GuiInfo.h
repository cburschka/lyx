// -*- C++ -*-
/**
 * \file GuiInfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUI_INFO_H
#define GUI_INFO_H

#include "InsetParamsWidget.h"
#include "ui_InfoUi.h"

namespace lyx {

class Inset;

namespace frontend {

class GuiInfo : public InsetParamsWidget, public Ui::InfoUi
{
	Q_OBJECT

public:
	GuiInfo(QWidget * parent = 0);

private:
	/// \name InsetParamsWidget inherited methods
	//@{
	InsetCode insetCode() const { return INFO_CODE; }
	FuncCode creationCode() const { return LFUN_INFO_INSERT; }
	QString dialogTitle() const { return qt_("Info Inset Settings"); }
	void paramsToDialog(Inset const *);
	docstring dialogToParams() const;
	bool checkWidgets(bool readonly) const;
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // GUI_INFO_H
