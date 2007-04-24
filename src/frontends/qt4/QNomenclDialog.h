// -*- C++ -*-
/**
 * \file QNomenclDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author O. U. Baran
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QNOMENCLDIALOG_H
#define QNOMENCLDIALOG_H

#include "ui/NomenclUi.h"

#include <QDialog>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

class QNomencl;

class QNomenclDialog : public QDialog, public Ui::QNomenclUi {
	Q_OBJECT
public:
	QNomenclDialog(QNomencl * form);
	virtual void show();
protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void reject();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QNomencl * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QNOMENCLDIALOG_H
