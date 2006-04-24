// -*- C++ -*-
/**
 * \file QShowFileDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSHOWFILEDIALOG_H
#define QSHOWFILEDIALOG_H

#include "ui/QShowFileDialogBase.h"

namespace lyx {
namespace frontend {

class QShowFile;

class QShowFileDialog : public QShowFileDialogBase {
	Q_OBJECT
public:
	QShowFileDialog(QShowFile * form);
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QShowFile * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QSHOWFILEDIALOG_H
