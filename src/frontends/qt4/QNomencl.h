// -*- C++ -*-
/**
 * \file QNomencl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 * \author O. U. Baran
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QNOMENCL_H
#define QNOMENCL_H

#include "QDialogView.h"
#include "ui_NomenclUi.h"

#include <QDialog>

class QCloseEvent;


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


class ControlCommand;

class QNomencl :
	public QController<ControlCommand, QView<QNomenclDialog> >
{
public:
	friend class QNomenclDialog;

	QNomencl(Dialog &, docstring const & title);
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

#endif // QNOMENCL_H
