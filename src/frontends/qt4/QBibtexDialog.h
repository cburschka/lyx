// -*- C++ -*-
/**
 * \file QBibtexDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBIBTEXDIALOG_H
#define QBIBTEXDIALOG_H

#include "ui/QBibtexUi.h"
#include "ui/QBibtexAddUi.h"

#include "ButtonController.h"
#include "QBibtex.h"

#include <QCloseEvent>
#include <QDialog>

namespace lyx {
namespace frontend {

template<class UI>
	class UiDialog: public QDialog, public UI
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
	~QBibtexDialog();

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

} // namespace frontend
} // namespace lyx

#endif // QBIBTEXDIALOG_H
