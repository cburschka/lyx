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

#include "InsetDialog.h"
#include "ui_ERTUi.h"

namespace lyx {
namespace frontend {

class GuiERT : public InsetDialog, public Ui::ERTUi
{
	Q_OBJECT

public:
	GuiERT(GuiView & lv);

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

#endif // GUIERT_H
