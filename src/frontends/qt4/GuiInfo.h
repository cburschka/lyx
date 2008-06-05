// -*- C++ -*-
/**
 * \file GuiInfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUI_INFO_H
#define GUI_INFO_H

#include "DialogView.h"
#include "ui_InfoUi.h"

namespace lyx {

class InsetInfo;

namespace frontend {

class GuiInfo : public DialogView, public Ui::InfoUi
{
	Q_OBJECT

public:
	GuiInfo(GuiView & lv);

	/// Dialog inherited methods
	//@{
	void applyView();
	void updateView();
	void dispatchParams();
	void enableView(bool enable);
	bool isBufferDependent() const { return true; }
	//@}

private Q_SLOTS:
	void on_closePB_clicked();
};

} // namespace frontend
} // namespace lyx

#endif // GUI_INFO_H
