// -*- C++ -*-
/**
 * \file GuiTabularCreate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUITABULARCREATE_H
#define GUITABULARCREATE_H

#include "GuiDialog.h"
#include "ControlTabularCreate.h"
#include "ui_TabularCreateUi.h"

namespace lyx {
namespace frontend {

class GuiTabularCreateDialog : public GuiDialog, public Ui::TabularCreateUi
{
	Q_OBJECT

public:
	GuiTabularCreateDialog(LyXView & lv);

private Q_SLOTS:
	void columnsChanged(int);
	void rowsChanged(int);

	/// parent controller
	ControlTabularCreate & controller();

private:
	/// Apply changes
	void applyView();
};

} // namespace frontend
} // namespace lyx

#endif // GUITABULARCREATE_H
