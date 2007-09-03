// -*- C++ -*-
/**
 * \file GuiSendto.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUISENDTO_H
#define GUISENDTO_H

#include "GuiDialogView.h"
#include "ControlSendto.h"
#include "ui_SendtoUi.h"

#include <QDialog>

#include <vector>

class QListWidgetItem;

namespace lyx {

class Format;

namespace frontend {

class GuiSendto;

class GuiSendtoDialog : public QDialog, public Ui::SendtoUi {
	Q_OBJECT
public:
	GuiSendtoDialog(GuiSendto * form);
protected Q_SLOTS:
	virtual void changed_adaptor();
	virtual void slotFormatHighlighted(QListWidgetItem *) {}
	virtual void slotFormatSelected(QListWidgetItem *) {}
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	GuiSendto * form_;
};


/** This class provides a Qt implementation of the Custom Export Dialog.
 */
class GuiSendto : public GuiView<GuiSendtoDialog>
{
public:
	///
	friend class GuiSendtoDialog;
	///
	GuiSendto(GuiDialog &);
	/// parent controller
	ControlSendto & controller()
	{ return static_cast<ControlSendto &>(this->getController()); }
	/// parent controller
	ControlSendto const & controller() const
	{ return static_cast<ControlSendto const &>(this->getController()); }
protected:
	virtual bool isValid();
private:
	/// Apply from dialog
	virtual void apply();
	/// Update the dialog
	virtual void update_contents();
	/// Build the dialog
	virtual void build_dialog();
	///
	std::vector<Format const *> all_formats_;
};

} // namespace frontend
} // namespace lyx

#endif // GUISENDTO_H
