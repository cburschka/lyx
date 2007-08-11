// -*- C++ -*-
/**
 * \file QVSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Angus Leeming
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QVSPACE_H
#define QVSPACE_H

#include "QDialogView.h"

#include "ui_VSpaceUi.h"

#include <QDialog>

class QCloseEvent;

namespace lyx {
namespace frontend {

class ControlVSpace;

class QVSpace;


class QVSpaceDialog : public QDialog, public Ui::QVSpaceUi {
	Q_OBJECT

public:
	QVSpaceDialog(QVSpace * form);

public Q_SLOTS:
	void change_adaptor();

protected Q_SLOTS:
	void closeEvent(QCloseEvent *);
	void enableCustom(int);

private:
	QVSpace * form_;
};



class QVSpace
	: public QController<ControlVSpace, QView<QVSpaceDialog> >
{
public:
	///
	friend class QVSpaceDialog;
	///
	QVSpace(Dialog &);
private:
	/// Build the dialog
	virtual void build_dialog();
	/// Apply from dialog
	virtual void apply();
	/// Update the dialog
	virtual void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif //QVSPACE_H
