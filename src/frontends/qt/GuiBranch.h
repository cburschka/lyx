// -*- C++ -*-
/**
 * \file GuiBranch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIBRANCH_H
#define GUIBRANCH_H

#include "InsetParamsWidget.h"
#include "ui_BranchUi.h"

namespace lyx {
namespace frontend {

class GuiBranch : public InsetParamsWidget, public Ui::BranchUi
{
	Q_OBJECT

public:
	GuiBranch(QWidget * parent = 0);

private:
	/// \name DialogView inherited methods
	//@{
	InsetCode insetCode() const override { return BRANCH_CODE; }
	FuncCode creationCode() const override { return LFUN_BRANCH_INSERT; }
	QString dialogTitle() const override { return qt_("Branch Settings"); }
	void paramsToDialog(Inset const *) override;
	docstring dialogToParams() const override;
	bool checkWidgets(bool readonly) const override;
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // GUIBRANCH_H
