/**
 * \file copyrightdlg.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef COPYRIGHTDIALOG_H
#define COPYRIGHTDIALOG_H

#include "dlg/copyrightdlgdata.h"

class FormCopyright;
 
class CopyrightDialog : public CopyrightDialogData
{
	Q_OBJECT
 
public:
	CopyrightDialog (FormCopyright * f, QWidget * parent = NULL, char const * name = NULL);

	virtual ~CopyrightDialog();

protected slots:

	virtual void clickedOK();
	virtual void closeEvent(QCloseEvent * e);

private:
	FormCopyright * form_;
};
 
#endif // COPYRIGHTDIALOG_H
