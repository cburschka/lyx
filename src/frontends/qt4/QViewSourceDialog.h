// -*- C++ -*-
/**
 * \file QViewSourceDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QVIEWSOURCEDIALOG_H
#define QVIEWSOURCEDIALOG_H

#include "ui/QViewSourceUi.h"

namespace lyx {
namespace frontend {

class QViewSource;

class QViewSourceDialog : public QDialog, public Ui::QViewSourceUi {
	Q_OBJECT
public:
	QViewSourceDialog(QViewSource * form);
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QViewSource * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QVIEWSOURCEDIALOG_H
