// -*- C++ -*-
/**
 * \file QERT.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QERT_H
#define QERT_H

#include "QDialogView.h"
#include "ui_ERTUi.h"

#include <QCloseEvent>
#include <QDialog>

namespace lyx {
namespace frontend {

class QERT;

class QERTDialog : public QDialog, public Ui::QERTUi {
	Q_OBJECT
public:
	QERTDialog(QERT * form);
protected Q_SLOTS:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QERT * form_;
};



class ControlERT;

class QERT : public QController<ControlERT, QView<QERTDialog> >
{
public:
	friend class QERTDialog;

	QERT(Dialog &);
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

#endif // QERT_H
