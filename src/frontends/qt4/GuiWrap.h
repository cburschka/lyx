// -*- C++ -*-
/**
 * \file GuiWrap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIWRAP_H
#define GUIWRAP_H

#include "GuiDialog.h"
#include "ControlWrap.h"
#include "ui_WrapUi.h"

namespace lyx {
namespace frontend {

class GuiWrapDialog : public GuiDialog, public Ui::WrapUi
{
	Q_OBJECT

public:
	GuiWrapDialog(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();
	/// overhang checkbox
	void overhangChecked(int);
	/// lines checkbox
	void linesChecked(int);

private:
	void closeEvent(QCloseEvent * e);
	/// parent controller
	ControlWrap & controller();
	/// Apply changes
	void applyView();
	/// update
	void updateContents();
};

} // namespace frontend
} // namespace lyx

#endif // GUIWRAP_H
