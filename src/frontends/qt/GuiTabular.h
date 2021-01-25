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

#include "InsetParamsWidget.h"
#include "ui_TabularUi.h"
#include "insets/InsetTabular.h"

namespace lyx {
namespace frontend {

class GuiTabular : public InsetParamsWidget, public Ui::TabularUi
{
	Q_OBJECT

public:
	GuiTabular(QWidget * parent = 0);

private Q_SLOTS:
	void checkEnabled();
	void borderSet_clicked();
	void borderUnset_clicked();
	void booktabs_toggled(bool const check);
	void nonbooktabs_toggled(bool const check);
	void on_topspaceCO_activated(int index);
	void on_bottomspaceCO_activated(int index);
	void on_interlinespaceCO_activated(int index);
	void on_columnTypeCO_activated(int index);

private:
	/// \name InsetParamsWidget inherited methods
	//@{
	InsetCode insetCode() const override { return TABULAR_CODE; }
	FuncCode creationCode() const override { return LFUN_TABULAR_INSERT; }
	QString dialogTitle() const override { return qt_("Tabular Settings"); }
	void paramsToDialog(Inset const *) override;
	docstring dialogToParams() const override;
	bool checkWidgets(bool readonly) const override;
	//@}

	///
	void enableWidgets() const;
	///
	void setHAlign(std::set<std::string> & params) const;
	///
	void setVAlign(std::set<std::string> & params) const;
	///
	void setTableAlignment(std::set<std::string> & params) const;
	///
	std::set<std::string> const getTabFeatures() const;
	///
	void setWidthAndAlignment();
	///
	bool funcEnabled(Tabular::Feature f) const;
	///
	GuiSetBorder::BorderState borderState(GuiSetBorder::BorderState bs,
					      bool const line);
	///
	bool firstheader_suppressable_;
	///
	bool lastfooter_suppressable_;
	///
	GuiSetBorder::BorderState orig_leftborder_;
	///
	GuiSetBorder::BorderState orig_rightborder_;
	///
	int lastrow_;
	///
	docstring decimal_sep_;
	///
	std::set<std::string> features_;
};

} // namespace frontend
} // namespace lyx

#endif // GUITABULAR_H
