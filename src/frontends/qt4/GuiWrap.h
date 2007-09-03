// -*- C++ -*-
/**
 * \file GuiWrap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIWRAP_H
#define GUIWRAP_H

#include "GuiDialogView.h"
#include "ControlWrap.h"
#include "ui_WrapUi.h"

#include <QDialog>

namespace lyx {
namespace frontend {

class GuiWrap;

class GuiWrapDialog : public QDialog, public Ui::WrapUi
{
	Q_OBJECT
public:
	GuiWrapDialog(GuiWrap * form);
protected Q_SLOTS:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiWrap * form_;
};


class GuiWrap : public GuiView<GuiWrapDialog>
{
public:
	friend class GuiWrapDialog;

	GuiWrap(GuiDialog &);
	/// parent controller
	ControlWrap & controller()
	{ return static_cast<ControlWrap &>(this->getController()); }
	/// parent controller
	ControlWrap const & controller() const
	{ return static_cast<ControlWrap const &>(this->getController()); }
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // GUIWRAP_H
