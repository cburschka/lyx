/**
 * \file paraextradlg.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef PARAEXTRADIALOG_H
#define PARAEXTRADIALOG_H

#include "dlg/paraextradlgdata.h"

class ParaDialog;

class ParaExtraDialog : public ParaExtraDialogData
{
	Q_OBJECT

public:
	ParaExtraDialog (QWidget * parent = NULL, char const * name = NULL);

	virtual ~ParaExtraDialog();

	friend class ParaDialog;

protected slots:

	void typeHighlighted(int);

};
#endif // PARAEXTRADIALOG_H
