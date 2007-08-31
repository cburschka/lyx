// -*- C++ -*-
/**
 * \file GuiChanges.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUICHANGES_H
#define GUICHANGES_H

#include "GuiDialogView.h"
#include "ControlChanges.h"
#include "ui_ChangesUi.h"

#include <QDialog>

namespace lyx {
namespace frontend {

class GuiChanges;

class GuiChangesDialog : public QDialog, public Ui::ChangesUi {
	Q_OBJECT
public:

	GuiChangesDialog(GuiChanges * form);

protected Q_SLOTS:

	virtual void nextPressed();
	virtual void acceptPressed();
	virtual void rejectPressed();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	GuiChanges * form_;
};


class GuiChanges : public GuiView<GuiChangesDialog>
{
public:
	///
	GuiChanges(Dialog &);
	/// parent controller
	ControlChanges & controller()
	{ return static_cast<ControlChanges &>(this->getController()); }
	/// parent controller
	ControlChanges const & controller() const
	{ return static_cast<ControlChanges const &>(this->getController()); }

	void accept();

	void reject();

	void next();

private:
	friend class GuiChangesDialog;
	/// Apply changes
	virtual void apply() {};
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // GUICHANGES_H
