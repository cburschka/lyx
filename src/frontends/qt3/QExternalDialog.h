// -*- C++ -*-
/**
 * \file QExternalDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QEXTERNALDIALOG_H
#define QEXTERNALDIALOG_H

#include "ui/QExternalDialogBase.h"

namespace lyx {
namespace frontend {

class QExternal;

class QExternalDialog : public QExternalDialogBase {
	Q_OBJECT
public:
	QExternalDialog(QExternal * form);

	virtual void show();
protected slots:
	virtual void bbChanged();
	virtual void browseClicked();
	virtual void change_adaptor();
	virtual void editClicked();
	virtual void extraChanged(const QString&);
	virtual void formatChanged(const QString&);
	virtual void getbbClicked();
	virtual void sizeChanged();
	virtual void templateChanged();
	virtual void widthUnitChanged();

protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	bool activateAspectratio() const;
	QExternal * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QEXTERNALDIALOG_H
