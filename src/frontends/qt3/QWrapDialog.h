// -*- C++ -*-
/**
 * \file QWrapDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QWRAPDIALOG_H
#define QWRAPDIALOG_H

#include "ui/QWrapDialogBase.h"

namespace lyx {
namespace frontend {

class QWrap;

class QWrapDialog : public QWrapDialogBase {
	Q_OBJECT
public:
	QWrapDialog(QWrap * form);
protected slots:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QWrap * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QWRAPDIALOG_H
