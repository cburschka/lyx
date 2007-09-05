// -*- C++ -*-
/**
 * \file GuiAbout.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIABOUT_H
#define GUIABOUT_H

#include "GuiDialog.h"
#include "ControlAboutlyx.h"
#include "ui_AboutUi.h"

#include <QDialog>

namespace lyx {
namespace frontend {

class GuiAboutDialog : public GuiDialog, public Ui::AboutUi
{
	Q_OBJECT

public:
	// Constructor
	GuiAboutDialog(LyXView & lv);
	/// parent controller
	ControlAboutlyx & controller() const;
};

} // namespace frontend
} // namespace lyx

#endif // GUIABOUT_H
