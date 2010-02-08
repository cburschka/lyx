// -*- C++ -*-
/**
 * \file GuiFloat.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIFLOAT_H
#define GUIFLOAT_H

#include "InsetDialog.h"

#include "ui_FloatUi.h"


namespace lyx {
namespace frontend {

class GuiFloat : public InsetDialog, public Ui::FloatUi
{
	Q_OBJECT

public:
	GuiFloat(GuiView & lv);

private:
	/// \name Dialog inerited methods
	//@{
	void enableView(bool enable);
	//@}

	/// \name InsetDialog inherited methods
	//@{
	void paramsToDialog(Inset const *);
	docstring dialogToParams() const;
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // GUIFLOAT_H
