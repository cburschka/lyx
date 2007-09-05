// -*- C++ -*-
/**
 * \file GuiChanges.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUICHANGES_H
#define GUICHANGES_H

#include "GuiDialog.h"
#include "ControlChanges.h"
#include "ui_ChangesUi.h"

namespace lyx {
namespace frontend {

class GuiChangesDialog : public GuiDialog, public Ui::ChangesUi
{
	Q_OBJECT

public:
	GuiChangesDialog(LyXView & lv);

protected Q_SLOTS:
	void nextPressed();
	void acceptPressed();
	void rejectPressed();

private:
	void closeEvent(QCloseEvent * e);
	ControlChanges & controller() const;
	void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // GUICHANGES_H
