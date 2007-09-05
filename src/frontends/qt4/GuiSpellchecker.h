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

#include "GuiDialog.h"
#include "ControlSpellchecker.h"
#include "ui_SpellcheckerUi.h"

class QListWidgetItem;

namespace lyx {
namespace frontend {

class GuiSpellcheckerDialog : public GuiDialog, public Ui::SpellcheckerUi
{
	Q_OBJECT

public:
	GuiSpellcheckerDialog(LyXView & lv);

public Q_SLOTS:
	void suggestionChanged(QListWidgetItem *);

private Q_SLOTS:
	void acceptClicked();
	void addClicked();
	void replaceClicked();
	void ignoreClicked();
	void replaceChanged(const QString &);
	void reject();

private:
	///
	void closeEvent(QCloseEvent * e);
	/// update from controller
	void partialUpdate(int id);
	/// parent controller
	ControlSpellchecker & controller() const;
	////
	void accept();
	void add();
	void ignore();
	void replace();
	///
	void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // GUISPELLCHECKER_H
