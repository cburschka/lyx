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

#ifndef QCHANGES_H
#define QCHANGES_H

#include "GuiDialogView.h"
#include "ui_ChangesUi.h"
#include <QCloseEvent>
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

class ControlChanges;

class GuiChanges
	: public QController<ControlChanges, GuiView<GuiChangesDialog> >
{
public:
	friend class GuiChangesDialog;

	GuiChanges(Dialog &);

	void accept();

	void reject();

	void next();

private:
	/// Apply changes
	virtual void apply() {};
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // QCHANGES_H
