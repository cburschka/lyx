// -*- C++ -*-
/**
 * \file GuiPrint.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIPRINT_H
#define GUIPRINT_H

#include "GuiDialogView.h"
#include "ControlPrint.h"
#include "ui_PrintUi.h"

#include <QDialog>

namespace lyx {
namespace frontend {

class GuiPrint;

class GuiPrintDialog : public QDialog, public Ui::PrintUi {
	Q_OBJECT
public:
	GuiPrintDialog(GuiPrint * f);
protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void browseClicked();
	virtual void fileChanged();
	virtual void copiesChanged(int);
	virtual void printerChanged();
	virtual void pagerangeChanged();
private:
	GuiPrint * form_;
};


class GuiPrint : public GuiView<GuiPrintDialog>
{
public:
	///
	friend class GuiPrintDialog;
	///
	GuiPrint(GuiDialog &);
	/// parent controller
	ControlPrint & controller()
	{ return static_cast<ControlPrint &>(this->getController()); }
	/// parent controller
	ControlPrint const & controller() const
	{ return static_cast<ControlPrint const &>(this->getController()); }
private:
	/// Apply changes
	virtual void applyView();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // GUIPRINT_H
