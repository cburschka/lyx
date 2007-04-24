// -*- C++ -*-
/**
 * \file QIncludeDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QINCLUDEDIALOG_H
#define QINCLUDEDIALOG_H

#include "ui/IncludeUi.h"

#include <QDialog>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

class QInclude;

class QIncludeDialog : public QDialog, public Ui::QIncludeUi {
	Q_OBJECT
public:
	QIncludeDialog(QInclude * form);

	void updateLists();

	virtual void show();
protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void loadClicked();
	virtual void browseClicked();
	virtual void typeChanged(int v);
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QInclude * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QINCLUDEDIALOG_H
