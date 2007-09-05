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

#ifndef GUIPRINT_H
#define GUIPRINT_H

#include "GuiDialog.h"
#include "ControlPrint.h"
#include "ui_PrintUi.h"

namespace lyx {
namespace frontend {

class GuiPrintDialog : public GuiDialog, public Ui::PrintUi
{
	Q_OBJECT

public:
	GuiPrintDialog(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();
	void browseClicked();
	void fileChanged();
	void copiesChanged(int);
	void printerChanged();
	void pagerangeChanged();
	/// parent controller
	ControlPrint & controller() const;

private:
	/// Apply changes
	void applyView();
	/// update
	void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // GUIPRINT_H
