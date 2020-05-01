// -*- C++ -*-
/**
 * \file GuiCounter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Kimberly Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUICOUNTER_H
#define GUICOUNTER_H

#include "InsetParamsWidget.h"
#include "ui_CounterUi.h"
#include "GuiView.h"

namespace lyx {
class InsetCommandParams;

namespace frontend {

class GuiCounter : public InsetParamsWidget, public Ui::CounterUi
{
	Q_OBJECT

public:
	///
	GuiCounter(GuiView & lv, QWidget * parent = nullptr);

private:
	/// \name InsetParamsWidget inherited methods
	//@{
	InsetCode insetCode() const { return COUNTER_CODE; }
	FuncCode creationCode() const { return LFUN_INSET_INSERT; }
	QString dialogTitle() const { return qt_("Counters"); }
	void paramsToDialog(Inset const *);
	docstring dialogToParams() const;
	bool checkWidgets(bool readonly) const;
	bool initialiseParams(std::string const & data);
	//@}
	void processParams(InsetCommandParams const & icp);
	///
	void fillCombos();
	///
	GuiView & guiview;
};

} // namespace frontend
} // namespace lyx

#endif // GUICOUNTER_H
