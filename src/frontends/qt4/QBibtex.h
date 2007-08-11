// -*- C++ -*-
/**
 * \file QBibtex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBIBTEX_H
#define QBIBTEX_H

#include "QDialogView.h"

#include "ui_BibtexUi.h"
#include "ui_BibtexAddUi.h"

#include "ButtonController.h"

#include <QCloseEvent>
#include <QDialog>

namespace lyx {
namespace frontend {

class QBibtex;

template<class UI>
class UiDialog : public QDialog, public UI
{
public:
	UiDialog(QWidget * parent=0, bool modal=false, Qt::WFlags f=0)
		: QDialog(parent, f)
	{
		UI::setupUi(this);
		QDialog::setModal(modal);
	}
};


class QBibtexDialog : public QDialog, public Ui::QBibtexUi {
	Q_OBJECT

public:
	QBibtexDialog(QBibtex * form);

	UiDialog<Ui::QBibtexAddUi> * add_;

protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void browsePressed();
	virtual void browseBibPressed();
	virtual void addPressed();
	virtual void addDatabase();
	virtual void deletePressed();
	virtual void databaseChanged();
	virtual void availableChanged();
	void bibEDChanged();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QBibtex * form_;
	ButtonController add_bc_;
};


class ControlBibtex;

class QBibtex
	: public QController<ControlBibtex, QView<QBibtexDialog> >
{
public:
	friend class QBibtexDialog;

	QBibtex(Dialog &);
protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // QBIBTEX_H
