// -*- C++ -*-
/**
 * \file GuiURLDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIURLDIALOG_H
#define GUIURLDIALOG_H

#include "GuiDialog.h"
#include "ControlCommand.h"
#include "ui_URLUi.h"

namespace lyx {
namespace frontend {

class GuiURLDialog : public GuiDialog, public Ui::URLUi
{
	Q_OBJECT

public:
	GuiURLDialog(LyXView & lv);

public Q_SLOTS:
	void changed_adaptor();

private:
	void closeEvent(QCloseEvent *);
	/// parent controller
	ControlCommand & controller() const;
	///
	bool isValid();
	/// apply dialog
	void applyView();
	/// update dialog
	void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // GUIURLDIALOG_H
