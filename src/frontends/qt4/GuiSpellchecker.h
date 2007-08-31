// -*- C++ -*-
/**
 * \file GuiSpellchecker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUISPELLCHECKER_H
#define GUISPELLCHECKER_H

#include "GuiDialogView.h"
#include "ControlSpellchecker.h"
#include "ui_SpellcheckerUi.h"

#include <QDialog>

class QListWidgetItem;


namespace lyx {
namespace frontend {

class GuiSpellchecker;

class GuiSpellcheckerDialog: public QDialog, public Ui::SpellcheckerUi {
	Q_OBJECT
public:
	GuiSpellcheckerDialog(GuiSpellchecker * form);
public Q_SLOTS:
	virtual void suggestionChanged(QListWidgetItem *);

protected Q_SLOTS:
	virtual void acceptClicked();
	virtual void addClicked();
	virtual void replaceClicked();
	virtual void ignoreClicked();
	virtual void replaceChanged(const QString &);
	virtual void reject();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	GuiSpellchecker * form_;
};


class GuiSpellchecker : public GuiView<GuiSpellcheckerDialog>
{
public:
	friend class GuiSpellcheckerDialog;

	GuiSpellchecker(Dialog &);

	/// update from controller
	void partialUpdate(int id);
	/// parent controller
	ControlSpellchecker & controller()
	{ return static_cast<ControlSpellchecker &>(this->getController()); }
	/// parent controller
	ControlSpellchecker const & controller() const
	{ return static_cast<ControlSpellchecker const &>(this->getController()); }
private:
	void accept();
	void add();
	void ignore();
	void replace();

	/// Apply changes
	virtual void apply() {}
	///
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // GUISPELLCHECKER_H
