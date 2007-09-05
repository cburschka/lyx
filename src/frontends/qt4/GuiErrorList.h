// -*- C++ -*-
/**
 * \file GuiErrorList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIERRORLIST_H
#define GUIERRORLIST_H

#include "GuiDialog.h"
#include "ControlErrorList.h"
#include "ui_ErrorListUi.h"

class QListWidgetItem;

namespace lyx {
namespace frontend {

class GuiErrorListDialog : public GuiDialog, public Ui::ErrorListUi
{
	Q_OBJECT

public:
	GuiErrorListDialog(LyXView & lv);

public Q_SLOTS:
	void select_adaptor(QListWidgetItem *);

private:
	void closeEvent(QCloseEvent *);
	void showEvent(QShowEvent *);
	/// parent controller
	ControlErrorList & controller() const;
	/// select an entry
	void select(QListWidgetItem *);
	/// update contents
	void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // GUIERRORLIST_H
