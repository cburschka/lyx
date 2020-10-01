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
#include "insets/InsetInfo.h"
#include "ui_InfoUi.h"

namespace lyx {

class Inset;

namespace frontend {

class GuiInfo : public InsetParamsWidget, public Ui::InfoUi
{
	Q_OBJECT

protected Q_SLOTS:
	void updateArguments(int i);

public:
	GuiInfo(QWidget * parent = 0);

private:
	/// \name InsetParamsWidget inherited methods
	//@{
	InsetCode insetCode() const override { return INFO_CODE; }
	FuncCode creationCode() const override { return LFUN_INFO_INSERT; }
	QString dialogTitle() const override { return qt_("Field Settings"); }
	void paramsToDialog(Inset const *) override;
	docstring dialogToParams() const override;
	bool initialiseParams(std::string const &) override;
	bool checkWidgets(bool readonly) const override;
	//@}
	InsetInfoParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUI_INFO_H
