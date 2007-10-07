// -*- C++ -*-
/**
 * \file GuiURL.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIURL_H
#define GUIURL_H

#include "GuiDialog.h"
#include "ui_URLUi.h"
#include "insets/InsetCommandParams.h"

namespace lyx {
namespace frontend {

class GuiURL : public GuiCommand, public Ui::URLUi
{
	Q_OBJECT

public:
	GuiURL(LyXView & lv);

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

#endif // GUIURL_H
