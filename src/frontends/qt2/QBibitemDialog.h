// -*- C++ -*-
/**
 * \file QBibitemDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBIBITEMDIALOG_H
#define QBIBITEMDIALOG_H

#include "ui/QBibitemDialogBase.h"

namespace lyx {
namespace frontend {

class QBibitem;

class QBibitemDialog : public QBibitemDialogBase {
	Q_OBJECT
public:
	QBibitemDialog(QBibitem * form);
protected slots:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QBibitem * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QBIBITEMDIALOG_H
