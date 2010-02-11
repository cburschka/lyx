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
#include "ui_TabularUi.h"
#include "insets/InsetTabular.h"

namespace lyx {
namespace frontend {

class GuiTabular : public GuiDialog, public Ui::TabularUi
{
	Q_OBJECT

public:
	GuiTabular(GuiView & lv);
	~GuiTabular();

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
	void multirow_clicked();
	void rotateTabular();
	void rotateCell();
	void hAlign_changed(int align);
	void vAlign_changed(int align);
	void tableAlignment_changed(int align);
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
	void ltAlignment_clicked();
	void on_captionStatusCB_toggled();

private:
	///
	bool isValid() { return true; }
	/// update borders
	void updateBorders(Tabular const & tabular);
	/// update
	void updateContents();
	///
	void paramsToDialog(Tabular const & tabular);
	/// save some values before closing the gui
	void closeGUI(Tabular const & tabular);
	///
	bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	void clearParams();
	/// We use set() instead.
	void dispatchParams() {};
	///
	bool isBufferDependent() const { return true; }
	///
	FuncCode getLfun() const { return LFUN_TABULAR_FEATURE; }

	///
	Tabular::idx_type getActiveCell() const;
	/// set a parameter
	void set(Tabular::Feature, std::string const & arg = std::string());

	void setSpecial(Tabular const & tabular, std::string const & special);

	void setWidth(Tabular const & tabular, std::string const & width);

	void toggleMultiColumn();
	void toggleMultiRow();

	void rotateTabular(bool yes);
	void rotateCell(bool yes);

	enum HALIGN { LEFT, RIGHT, CENTER, BLOCK };

	void halign(Tabular const & tabular, HALIGN h);

	enum VALIGN { TOP, MIDDLE, BOTTOM };

	void valign(Tabular const & tabular, VALIGN h);

	void booktabs(bool yes);

	void longTabular(bool yes);

	bool funcEnabled(Tabular::Feature f) const;

	///
	Tabular::idx_type active_cell_;
	///
	Tabular tabular_;
};

} // namespace frontend
} // namespace lyx

#endif // GUITABULAR_H
