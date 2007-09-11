// -*- C++ -*-
/**
 * \file GuiBibtex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIBIBTEX_H
#define GUIBIBTEX_H

#include "GuiDialog.h"
#include "ControlBibtex.h"
#include "ButtonController.h"
#include "ui_BibtexUi.h"
#include "ui_BibtexAddUi.h"

namespace lyx {
namespace frontend {

class GuiBibtexAddDialog : public QDialog, public Ui::BibtexAddUi
{
public:
	GuiBibtexAddDialog(QWidget * parent) : QDialog(parent)
	{
		Ui::BibtexAddUi::setupUi(this);
		QDialog::setModal(true);
	}
};


class GuiBibtexDialog : public GuiDialog, public Ui::BibtexUi
{
	Q_OBJECT

public:
	GuiBibtexDialog(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();
	void browsePressed();
	void browseBibPressed();
	void addPressed();
	void addDatabase();
	void deletePressed();
	void databaseChanged();
	void availableChanged();
	void bibEDChanged();

private:
	void closeEvent(QCloseEvent * e);

private:
	/// parent controller
	ControlBibtex & controller();
	///
	virtual bool isValid();
	/// Apply changes
	virtual void applyView();
	/// update
	virtual void updateContents();

	///
	GuiBibtexAddDialog * add_;
	///
	ButtonController add_bc_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIBIBTEX_H
