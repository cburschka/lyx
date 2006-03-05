// -*- C++ -*-
/**
 * \file QCitationDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QCITATIONDIALOG_H
#define QCITATIONDIALOG_H

#include "ui/QCitationUi.h"
#include "ui/QCitationFindUi.h"
#include "controllers/biblio.h"
#include <QDialog>

namespace lyx {
namespace frontend {

class QCitation;

class QCitationDialog : public QDialog, public Ui::QCitationUi {
	Q_OBJECT

public:
	QCitationDialog(QCitation * form);

	~QCitationDialog();

	void setButtons();
	/// open the find dialog if nothing selected
	void openFind();

	Ui::QCitationFindUi ui_;
	QDialog * add_;

protected slots:

	virtual void availableChanged();
	virtual void selectedChanged();
	virtual void up();
	virtual void down();
	virtual void del();
	virtual void addCitation();
	virtual void add();
	virtual void previous();
	virtual void next();
	virtual void changed_adaptor();

private:
	void find(biblio::Direction dir);

	QCitation * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QCITATIOINDIALOG_H
