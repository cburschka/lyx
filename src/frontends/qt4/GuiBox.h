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

#include "GuiDialog.h"
#include "ui_BoxUi.h"
#include "insets/InsetBox.h"


namespace lyx {
namespace frontend {

class GuiBox : public GuiDialog, public Ui::BoxUi
{
	Q_OBJECT

public:
	GuiBox(GuiView & lv);

private Q_SLOTS:
	void change_adaptor();
	void innerBoxChanged(QString const &);
	void typeChanged(int);
	void restoreClicked();
	void pagebreakClicked();

private:
	/// add and remove special lengths
	void setSpecial(bool ibox);
	/// only show valid inner box items
	void setInnerType(bool frameless, int i);

	/// Apply changes
	void applyView();
	/// update
	void updateContents();

	///
	bool initialiseParams(std::string const & data);
	///
	void clearParams();
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }

	///
	QStringList ids_;
	///
	QStringList gui_names_;
	///
	QStringList ids_spec_;
	///
	QStringList gui_names_spec_;

	///
	InsetBoxParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIBOX_H
