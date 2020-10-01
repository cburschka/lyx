// -*- C++ -*-
/**
 * \file GuiERT.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIERT_H
#define GUIERT_H

#include "InsetParamsWidget.h"
#include "ui_ERTUi.h"

namespace lyx {
namespace frontend {

class GuiERT : public InsetParamsWidget, public Ui::ERTUi
{
	Q_OBJECT

public:
	GuiERT(QWidget * parent = 0);

private:
	/// \name InsetParamsWidget inherited methods
	//@{
	InsetCode insetCode() const override { return ERT_CODE; }
	FuncCode creationCode() const override { return LFUN_INSET_INSERT; }
	QString dialogTitle() const override { return qt_("TeX Mode Inset Settings"); }
	void paramsToDialog(Inset const *) override;
	docstring dialogToParams() const override;
	bool checkWidgets(bool readonly) const override;
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // GUIERT_H
