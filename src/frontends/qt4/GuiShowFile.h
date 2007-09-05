// -*- C++ -*-
/**
 * \file GuiShowFile.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUISHOWFILE_H
#define GUISHOWFILE_H

#include "GuiDialog.h"
#include "ControlShowFile.h"
#include "ui_ShowFileUi.h"

namespace lyx {
namespace frontend {

class GuiShowFileDialog : public GuiDialog, public Ui::ShowFileUi
{
	Q_OBJECT

public:
	GuiShowFileDialog(LyXView & lv);

private:
	///
	void closeEvent(QCloseEvent * e);
	/// parent controller
	ControlShowFile & controller() const;
	/// update
	void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // GUISHOWFILE_H
