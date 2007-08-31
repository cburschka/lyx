// -*- C++ -*-
/**
 * \file GuiFloat.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIFLOAT_H
#define GUIFLOAT_H

#include "GuiDialogView.h"
#include "ui_FloatUi.h"
#include "ControlFloat.h"

#include <QDialog>

namespace lyx {
namespace frontend {

class GuiFloat;

class GuiFloatDialog : public QDialog, public Ui::FloatUi {
	Q_OBJECT
public:
	GuiFloatDialog(GuiFloat * form);

protected Q_SLOTS:
	virtual void change_adaptor();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	GuiFloat * form_;
};


///
class GuiFloat : public GuiView<GuiFloatDialog> {
public:
	///
	friend class GuiFloatDialog;
	///
	GuiFloat(Dialog &);
	/// parent controller
	ControlFloat & controller()
	{ return static_cast<ControlFloat &>(this->getController()); }
	/// parent controller
	ControlFloat const & controller() const
	{ return static_cast<ControlFloat const &>(this->getController()); }
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

#endif // GUIFLOAT_H
