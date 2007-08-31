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

#ifndef QFLOAT_H
#define QFLOAT_H

#include "GuiDialogView.h"
#include "GuiFloatDialog.h"

namespace lyx {
namespace frontend {


class ControlFloat;

///
class GuiFloat : public QController<ControlFloat, GuiView<GuiFloatDialog> > {
public:
	///
	friend class GuiFloatDialog;
	///
	GuiFloat(Dialog &);
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

#endif // QFLOAT_H
