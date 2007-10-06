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
#include "ui_AboutUi.h"

namespace lyx {
namespace frontend {

class GuiAbout : public GuiDialog, public Ui::AboutUi, public Controller
{
	Q_OBJECT

public:
	// Constructor
	GuiAbout(LyXView & lv);

	// Controller stuff
	bool initialiseParams(std::string const &) { return true; }
	void clearParams() {}
	void dispatchParams() {}
	bool isBufferDependent() const { return false; }
};

} // namespace frontend
} // namespace lyx

#endif // GUIABOUT_H
