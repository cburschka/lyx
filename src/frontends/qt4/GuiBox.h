// -*- C++ -*-
/**
 * \file GuiBox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 * \ author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIBOX_H
#define GUIBOX_H

#include "GuiDialog.h"
#include "ControlBox.h"
#include "ui_BoxUi.h"

#include <vector>

namespace lyx {
namespace frontend {

class GuiBoxDialog : public GuiDialog, public Ui::BoxUi
{
	Q_OBJECT

public:
	GuiBoxDialog(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();
	void innerBoxChanged(const QString &);
	void typeChanged(int);
	void restoreClicked();

private:
	void closeEvent(QCloseEvent * e);

	/// parent controller
	ControlBox & controller() const;
	/// add and remove special lengths
	void setSpecial(bool ibox);
	/// only show valid inner box items
	void setInnerType(bool frameless, int i);

	/// Apply changes
	void applyView();
	/// update
	void update_contents();

	///
	std::vector<std::string> ids_;
	///
	std::vector<docstring> gui_names_;
	///
	std::vector<std::string> ids_spec_;
	///
	std::vector<docstring> gui_names_spec_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIBOX_H
