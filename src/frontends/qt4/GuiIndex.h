// -*- C++ -*-
/**
 * \file GuiIndex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIINDEX_H
#define GUIINDEX_H

#include "GuiDialogView.h"
#include "ControlCommand.h"
#include "ui_IndexUi.h"

#include <QDialog>

namespace lyx {
namespace frontend {

class GuiIndex;

class GuiIndexDialog : public QDialog, public Ui::IndexUi {
	Q_OBJECT
public:
	GuiIndexDialog(GuiIndex * form);

protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void reject();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiIndex * form_;
};


class GuiIndex : public GuiView<GuiIndexDialog>
{
public:
	friend class GuiIndexDialog;

	GuiIndex(GuiDialog &, docstring const & title, QString const & label);
	/// parent controller
	ControlCommand & controller()
	{ return static_cast<ControlCommand &>(this->getController()); }
	/// parent controller
	ControlCommand const & controller() const
	{ return static_cast<ControlCommand const &>(this->getController()); }
protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	///
	QString const label_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIINDEX_H
