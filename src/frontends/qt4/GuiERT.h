// -*- C++ -*-
/**
 * \file GuiERT.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIERT_H
#define GUIERT_H

#include "GuiDialogView.h"
#include "ControlERT.h"
#include "ui_ERTUi.h"

#include <QCloseEvent>
#include <QDialog>

namespace lyx {
namespace frontend {

class GuiERT;

class GuiERTDialog : public QDialog, public Ui::ERTUi
{
	Q_OBJECT
public:
	GuiERTDialog(GuiERT * form);
protected Q_SLOTS:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiERT * form_;
};


class GuiERT : public GuiView<GuiERTDialog>
{
public:
	/// constructor
	GuiERT(Dialog &);
	/// parent controller
	ControlERT & controller()
	{ return static_cast<ControlERT &>(this->getController()); }
	/// parent controller
	ControlERT const & controller() const
	{ return static_cast<ControlERT const &>(this->getController()); }
private:
	friend class GuiERTDialog;
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // GUIERT_H
