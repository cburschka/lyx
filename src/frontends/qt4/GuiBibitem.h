// -*- C++ -*-
/**
 * \file GuiBibitem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIBIBITEM_H
#define GUIBIBITEM_H

#include "GuiDialogView.h"
#include "ControlCommand.h"
#include "ui_BibitemUi.h"

#include <QDialog>


namespace lyx {
namespace frontend {

class GuiBibitem;

class GuiBibitemDialog : public QDialog, public Ui::BibitemUi {
	Q_OBJECT
public:

	GuiBibitemDialog(GuiBibitem * form);
protected Q_SLOTS:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiBibitem * form_;
};


class GuiBibitem : public GuiView<GuiBibitemDialog>
{
public:
	///
	GuiBibitem(GuiDialog &);
	/// parent controller
	ControlCommand & controller()
	{ return static_cast<ControlCommand &>(this->getController()); }
	/// parent controller
	ControlCommand const & controller() const
	{ return static_cast<ControlCommand const &>(this->getController()); }
protected:
	virtual bool isValid();
private:
	friend class GuiBibitemDialog;
	/// Apply changes
	virtual void applyView();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // GUIBIBITEM_H
