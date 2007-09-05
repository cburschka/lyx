// -*- C++ -*-
/**
 * \file GuiVSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Angus Leeming
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIVSPACE_H
#define GUIVSPACE_H

#include "GuiDialog.h"
#include "ControlVSpace.h"
#include "ui_VSpaceUi.h"

namespace lyx {
namespace frontend {

class GuiVSpaceDialog : public GuiDialog, public Ui::VSpaceUi
{
	Q_OBJECT

public:
	GuiVSpaceDialog(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();
	void closeEvent(QCloseEvent *);
	void enableCustom(int);

private:
	/// parent controller
	ControlVSpace & controller() const;
	/// Apply from dialog
	void applyView();
	/// Update the dialog
	void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // GUIVSPACE_H
