// -*- C++ -*-
/**
 * \file GuiBox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 * \author Martin Vermeer (with useful hints from Angus Leeming)
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIBOX_H
#define GUIBOX_H

#include "InsetParamsWidget.h"
#include "ui_BoxUi.h"
#include "Font.h"

namespace lyx {
namespace frontend {

class GuiBox : public InsetParamsWidget, public Ui::BoxUi
{
	Q_OBJECT

public:
	GuiBox(QWidget * parent = 0);

private Q_SLOTS:
	void on_innerBoxCO_activated(int);
	void on_typeCO_activated(int);
	void on_frameColorCO_currentIndexChanged(int);
	void initDialog();
	void on_widthCB_stateChanged(int state);
	void on_heightCB_stateChanged(int state);
	void on_pagebreakCB_stateChanged();

private:
	/// \name DialogView inherited methods
	//@{
	InsetCode insetCode() const { return BOX_CODE; }
	FuncCode creationCode() const { return LFUN_BOX_INSERT; }
	QString dialogTitle() const { return qt_("Box Settings"); }
	void paramsToDialog(Inset const *);
	docstring dialogToParams() const;
	bool checkWidgets(bool readonly) const;
	//@}

	/// Fill the color combos
	void fillComboColor(QComboBox * combo, bool const is_none);
	/// add and remove special lengths
	void setSpecial(bool ibox);
	/// only show valid inner box items
	void setInnerType(bool frameless, QString const & type);

	QStringList ids_;
	///
	QStringList gui_names_;
	///
	QStringList ids_spec_;
	///
	QStringList gui_names_spec_;
	///
	QList<ColorCode> color_codes_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIBOX_H
