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

#include "ui/QBibtexDialogBase.h"
#include "ButtonController.h"

class QBibtexAddDialogBase;

namespace lyx {
namespace frontend {

class QBibtex;

class QBibtexDialog : public QBibtexDialogBase {
	Q_OBJECT

public:
	QBibtexDialog(QBibtex * form);
	~QBibtexDialog();

	QBibtexAddDialogBase * add_;

protected slots:
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
