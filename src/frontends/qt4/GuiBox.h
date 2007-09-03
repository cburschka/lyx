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

#include "GuiDialogView.h"
#include "ControlBox.h"
#include "ui_BoxUi.h"

#include <QDialog>

#include <vector>


namespace lyx {
namespace frontend {

class GuiBox;

class GuiBoxDialog : public QDialog, public Ui::BoxUi {
	Q_OBJECT
public:
	GuiBoxDialog(GuiBox * form);
protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void innerBoxChanged(const QString &);
	virtual void typeChanged(int);
	virtual void restoreClicked();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiBox * form_;
};


class GuiBox : public GuiView<GuiBoxDialog>
{
public:
	///
	GuiBox(GuiDialog &);
	/// parent controller
	ControlBox & controller()
	{ return static_cast<ControlBox &>(this->getController()); }
	/// parent controller
	ControlBox const & controller() const
	{ return static_cast<ControlBox const &>(this->getController()); }
	/// add and remove special lengths
	void setSpecial(bool ibox);
	/// only show valid inner box items
	void setInnerType(bool frameless, int i);
private:
	///
	friend class GuiBoxDialog;
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
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
