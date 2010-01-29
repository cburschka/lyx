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
	void borderSet_clicked();
	void borderUnset_clicked();

private:
	///
	bool isValid() { return true; }
	/// update borders
	void update_borders();
	///
	void applyView();
	/// update
	void updateContents();
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

	bool funcEnabled(Tabular::Feature f) const;

	///
	Tabular::idx_type active_cell_;
	///
	Tabular tabular_;
	///
	bool applying_;
};

} // namespace frontend
} // namespace lyx

#endif // GUITABULAR_H
