// -*- C++ -*-
/**
 * \file GuiTabular.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 * \author Jürgen Spitzmüller
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUITABULAR_H
#define GUITABULAR_H

#include "GuiDialog.h"
#include "ControlTabular.h"
#include "ui_TabularUi.h"

namespace lyx {
namespace frontend {

class GuiTabularDialog : public GuiDialog, public Ui::TabularUi
{
	Q_OBJECT

public:
	GuiTabularDialog(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();

	void topspace_changed();
	void bottomspace_changed();
	void interlinespace_changed();
	void booktabsChanged(bool);
	void close_clicked();
	void borderSet_clicked();
	void borderUnset_clicked();
	void leftBorder_changed();
	void rightBorder_changed();
	void topBorder_changed();
	void bottomBorder_changed();
	void multicolumn_clicked();
	void rotateTabular();
	void rotateCell();
	void hAlign_changed(int align);
	void vAlign_changed(int align);
	void specialAlignment_changed();
	void width_changed();
	void longTabular();
	void ltNewpage_clicked();
	void ltHeaderStatus_clicked();
	void ltHeaderBorderAbove_clicked();
	void ltHeaderBorderBelow_clicked();
	void ltFirstHeaderStatus_clicked();
	void ltFirstHeaderBorderAbove_clicked();
	void ltFirstHeaderBorderBelow_clicked();
	void ltFirstHeaderEmpty_clicked();
	void ltFooterStatus_clicked();
	void ltFooterBorderAbove_clicked();
	void ltFooterBorderBelow_clicked();
	void ltLastFooterStatus_clicked();
	void ltLastFooterBorderAbove_clicked();
	void ltLastFooterBorderBelow_clicked();
	void ltLastFooterEmpty_clicked();

private:
	///
	void closeEvent(QCloseEvent * e);
	/// parent controller
	ControlTabular & controller() const;
	///
	bool isValid() { return true; }
	/// update borders
	void update_borders();
	/// update
	void update_contents();
	/// save some values before closing the gui
	void closeGUI();
};

} // namespace frontend
} // namespace lyx

#endif // GUITABULAR_H
