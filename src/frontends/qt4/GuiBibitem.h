// -*- C++ -*-
/**
 * \file GuiBibitem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIBIBITEM_H
#define GUIBIBITEM_H

#include "GuiDialog.h"
#include "ui_BibitemUi.h"

#include "insets/InsetCommandParams.h"

namespace lyx {
namespace frontend {

class GuiBibitem : public GuiCommand, public Ui::BibitemUi
{
	Q_OBJECT

public:
	GuiBibitem(GuiView & lv);

private Q_SLOTS:
	void change_adaptor();

private:
	///
	void closeEvent(QCloseEvent * e);

private:
	///
	bool isValid();
	/// Apply changes
	void applyView();
	/// update
	void updateContents();
};

} // namespace frontend
} // namespace lyx

#endif // GUIBIBITEM_H
