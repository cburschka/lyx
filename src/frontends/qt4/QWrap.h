// -*- C++ -*-
/**
 * \file QWrap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QWRAP_H
#define QWRAP_H

#include "QDialogView.h"
#include "ui_WrapUi.h"

#include <QDialog>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

class QWrap;

class QWrapDialog : public QDialog, public Ui::QWrapUi {
	Q_OBJECT
public:
	QWrapDialog(QWrap * form);
protected Q_SLOTS:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QWrap * form_;
};


class ControlWrap;

class QWrap : public QController<ControlWrap, QView<QWrapDialog> > {
public:
	friend class QWrapDialog;

	QWrap(Dialog &);
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

#endif // QWRAP_H
