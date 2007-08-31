// -*- C++ -*-
/**
 * \file GuiURLDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIURLDIALOG_H
#define GUIURLDIALOG_H

#include "GuiDialogView.h"
#include "ControlCommand.h"
#include "ui_URLUi.h"

#include <QDialog>

namespace lyx {
namespace frontend {

class UrlView;

class GuiURLDialog : public QDialog, public Ui::URLUi
{
	Q_OBJECT
public:
	GuiURLDialog(UrlView * form);
public Q_SLOTS:
	void changed_adaptor();
protected:
	void closeEvent(QCloseEvent *);
private:
	UrlView * form_;
};


class UrlView : public GuiView<GuiURLDialog> 
{
public:
	friend class QURLDialog;
	UrlView(Dialog &);
	/// parent controller
	ControlCommand & controller()
	{ return static_cast<ControlCommand &>(this->getController()); }
	/// parent controller
	ControlCommand const & controller() const
	{ return static_cast<ControlCommand const &>(this->getController()); }
protected:
	virtual bool isValid();
private:
	/// apply dialog
	virtual void apply();
	/// build dialog
	virtual void build_dialog();
	/// update dialog
	virtual void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // GUIURLDIALOG_H
