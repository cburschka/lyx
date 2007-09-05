// -*- C++ -*-
/**
 * \file GuiSendto.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUISENDTO_H
#define GUISENDTO_H

#include "GuiDialog.h"
#include "ControlSendto.h"
#include "ui_SendtoUi.h"

#include <vector>

class QListWidgetItem;

namespace lyx {

class Format;

namespace frontend {

class GuiSendtoDialog : public GuiDialog, public Ui::SendtoUi
{
	Q_OBJECT

public:
	GuiSendtoDialog(LyXView & lv);

private Q_SLOTS:
	void changed_adaptor();
	void slotFormatHighlighted(QListWidgetItem *) {}
	void slotFormatSelected(QListWidgetItem *) {}

private:
	void closeEvent(QCloseEvent * e);
	/// parent controller
	ControlSendto & controller() const;
	///
	bool isValid();
	/// Apply from dialog
	void applyView();
	/// Update the dialog
	void update_contents();

	///
	std::vector<Format const *> all_formats_;
};

} // namespace frontend
} // namespace lyx

#endif // GUISENDTO_H
