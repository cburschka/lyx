// -*- C++ -*-
/**
 * \file GuiLabel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUILABEL_H
#define GUILABEL_H

#include "ui_LabelUi.h"

#include "InsetParamsWidget.h"

namespace lyx {
namespace frontend {

class GuiLabel : public InsetParamsWidget, public Ui::LabelUi
{
	Q_OBJECT

public:
	GuiLabel(QWidget * parent = 0);

private:
	/// \name InsetParamsWidget inherited methods
	//@{
	InsetCode insetCode() const { return LABEL_CODE; }
	FuncCode creationCode() const { return LFUN_INSET_INSERT; }
	void paramsToDialog(Inset const *);
	docstring dialogToParams() const;
	bool checkWidgets(bool readonly) const;
	bool initialiseParams(std::string const &);
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // GUILABEL_H
