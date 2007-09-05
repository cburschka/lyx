// -*- C++ -*-
/**
 * \file GuiERT.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIERT_H
#define GUIERT_H

#include "GuiDialog.h"
#include "ControlERT.h"
#include "ui_ERTUi.h"

namespace lyx {
namespace frontend {

class GuiERTDialog : public GuiDialog, public Ui::ERTUi
{
	Q_OBJECT

public:
	GuiERTDialog(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();

private:
	void closeEvent(QCloseEvent * e);
	/// parent controller
	ControlERT & controller() const;
	/// Apply changes
	void applyView();
	/// update
	void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // GUIERT_H
