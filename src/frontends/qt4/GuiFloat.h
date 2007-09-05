// -*- C++ -*-
/**
 * \file GuiFloat.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIFLOAT_H
#define GUIFLOAT_H

#include "GuiDialog.h"
#include "ControlFloat.h"
#include "ui_FloatUi.h"

namespace lyx {
namespace frontend {

class GuiFloatDialog : public GuiDialog, public Ui::FloatUi
{
	Q_OBJECT

public:
	GuiFloatDialog(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();

private:
	void closeEvent(QCloseEvent * e);
	/// parent controller
	ControlFloat & controller() const;
	/// Apply changes
	void applyView();
	/// update
	void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // GUIFLOAT_H
