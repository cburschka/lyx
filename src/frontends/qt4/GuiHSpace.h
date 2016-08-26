// -*- C++ -*-
/**
 * \file GuiHSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIHSPACE_H
#define GUIHSPACE_H

#include "InsetParamsWidget.h"
#include "ui_HSpaceUi.h"

namespace lyx {
namespace frontend {

class GuiHSpace : public InsetParamsWidget, public Ui::HSpaceUi
{
	Q_OBJECT

public:
	GuiHSpace(bool math_mode, QWidget * parent = 0);

private Q_SLOTS:
	///
	void changedSlot();
	///
	void enableWidgets() const;

private:
	/// \name InsetParamsWidget inherited methods
	//@{
	InsetCode insetCode() const { return math_mode_ ? MATH_SPACE_CODE : SPACE_CODE; }
	FuncCode creationCode() const { return LFUN_INSET_INSERT; }
	QString dialogTitle() const { return qt_("Horizontal Space Settings"); }
	void paramsToDialog(Inset const *);
	docstring dialogToParams() const;
	bool checkWidgets(bool readonly) const;
	//@}
	///
	bool const math_mode_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIHSPACE_H
