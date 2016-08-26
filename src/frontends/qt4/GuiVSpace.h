// -*- C++ -*-
/**
 * \file GuiVSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Angus Leeming
 * \author Jürgen Spitzmüller
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIVSPACE_H
#define GUIVSPACE_H

#include "InsetParamsWidget.h"
#include "ui_VSpaceUi.h"

namespace lyx {
namespace frontend {

class GuiVSpace : public InsetParamsWidget, public Ui::VSpaceUi
{
	Q_OBJECT

public:
	GuiVSpace(QWidget * parent = 0);

private Q_SLOTS:
	///
	void enableCustom(int);

private:
	/// \name InsetParamsWidget inherited methods
	//@{
	InsetCode insetCode() const { return VSPACE_CODE; }
	FuncCode creationCode() const { return LFUN_INSET_INSERT; }
	QString dialogTitle() const { return qt_("Vertical Space Settings"); }
	void paramsToDialog(Inset const *);
	docstring dialogToParams() const;
	bool checkWidgets(bool readonly) const;
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // GUIVSPACE_H
