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

// FIXME: closeEvent 
class FormCopyright;
 
class CopyrightDialog : public CopyrightDialogData
{
	Q_OBJECT
 
public:
	CopyrightDialog (FormCopyright * f, QWidget * parent = NULL, char const * name = NULL);

	virtual ~CopyrightDialog();

protected slots:

	virtual void clickedOK();

private:
	FormCopyright * form_;
};
 
#endif // COPYRIGHTDIALOG_H
