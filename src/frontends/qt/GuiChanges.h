// -*- C++ -*-
/**
 * \file GuiChanges.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Michael Gerz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUICHANGES_H
#define GUICHANGES_H

#include "GuiDialog.h"
#include "ui_ChangesUi.h"

#include "support/debug.h"
#include "support/docstring.h"


namespace lyx {
namespace frontend {

class GuiChanges : public GuiDialog, public Ui::ChangesUi
{
	Q_OBJECT

public:
	GuiChanges(GuiView & lv);

protected Q_SLOTS:
	/// accept the current change
	void acceptChange();
	/// reject the current change
	void rejectChange();
	/// find the next change and highlight it
	void nextChange();
	/// find the previous change and highlight it
	void previousChange();

private:
	///
	void updateContents();

	/// Nothing to initialise in this case.
	bool initialiseParams(std::string const &) { return true; }
	///
	void clearParams() {}
	///
	void dispatchParams() {}
	///
	bool isBufferDependent() const { return true; }
	/// always true since dispatchParams() is empty
	bool canApply() const { return true; }
};

} // namespace frontend
} // namespace lyx

#endif // GUICHANGES_H
