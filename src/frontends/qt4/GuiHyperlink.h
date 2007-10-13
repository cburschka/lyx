// -*- C++ -*-
/**
 * \file GuiHyperlink.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIHYPERLINK_H
#define GUIHYPERLINK_H

#include "GuiDialog.h"
#include "ui_HyperlinkUi.h"
#include "insets/InsetCommandParams.h"

namespace lyx {
namespace frontend {

class GuiHyperlink : public GuiCommand, public Ui::HyperlinkUi
{
	Q_OBJECT

public:
	GuiHyperlink(LyXView & lv);

public Q_SLOTS:
	void changed_adaptor();

private:
	void closeEvent(QCloseEvent *);
	///
	bool isValid();
	/// apply dialog
	void applyView();
	/// update dialog
	void updateContents();
};

} // namespace frontend
} // namespace lyx

#endif // GUIHYPERLINK_H
