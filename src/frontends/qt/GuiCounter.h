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

namespace lyx {
class InsetCommandParams;

namespace frontend {

class GuiView;

class GuiCounter : public InsetParamsWidget, public Ui::CounterUi
{
	Q_OBJECT

public:
	///
	GuiCounter(GuiView & lv, QWidget * parent = nullptr);

private:
	/// \name InsetParamsWidget inherited methods
	//@{
	InsetCode insetCode() const override { return COUNTER_CODE; }
	FuncCode creationCode() const override { return LFUN_INSET_INSERT; }
	QString dialogTitle() const override { return qt_("Counters"); }
	void paramsToDialog(Inset const *) override;
	docstring dialogToParams() const override;
	bool checkWidgets(bool readonly) const override;
	bool initialiseParams(std::string const & data) override;
	//@}
	void processParams(InsetCommandParams const & params);
	///
	void fillCombos();
	///
	GuiView & guiview;
};

} // namespace frontend
} // namespace lyx

#endif // GUICOUNTER_H
