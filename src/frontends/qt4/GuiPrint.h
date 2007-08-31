// -*- C++ -*-
/**
 * \file GuiPrint.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QPRINT_H
#define QPRINT_H

#include "GuiDialogView.h"
#include "GuiPrintDialog.h"

namespace lyx {
namespace frontend {

class ControlPrint;

///
class GuiPrint
	: public QController<ControlPrint, GuiView<GuiPrintDialog> >
{
public:
	///
	friend class GuiPrintDialog;
	///
	GuiPrint(Dialog &);
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // QPRINT_H
