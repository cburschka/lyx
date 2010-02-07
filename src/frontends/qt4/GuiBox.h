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

#include "InsetDialog.h"
#include "ui_BoxUi.h"
#include "insets/InsetBox.h"


namespace lyx {
namespace frontend {

class GuiBox : public InsetDialog, public Ui::BoxUi
{
	Q_OBJECT

public:
	GuiBox(GuiView & lv);

private Q_SLOTS:
	void on_innerBoxCO_activated(QString const &);
	void on_typeCO_activated(int);
	void initDialog();
	void on_heightCB_stateChanged(int state);
	void on_pagebreakCB_stateChanged();

private:
	/// \name Dialog inerited methods
	//@{
	void enableView(bool enable);
	//@}

	/// \name InsetDialog inherited methods
	//@{
	void paramsToDialog(Inset const *);
	docstring dialogToParams() const;
	//@}

	/// add and remove special lengths
	void setSpecial(bool ibox);
	/// only show valid inner box items
	void setInnerType(bool frameless, int i);

	QStringList ids_;
	///
	QStringList gui_names_;
	///
	QStringList ids_spec_;
	///
	QStringList gui_names_spec_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIBOX_H
