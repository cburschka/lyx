/**
 * \file logdlg.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef LOGDLG_H
#define LOGDLG_H

#include <config.h>
#include "support/lstrings.h"
 
#include "dlg/logdlgdata.h"

class DialogBase;

class LogDialog : public LogDialogData
{
	Q_OBJECT

public:
	LogDialog (DialogBase * f, QWidget * parent = NULL, char const * name = NULL);

	virtual ~LogDialog();

	void setLogText(string const & text);
private:
	DialogBase * form_;
protected slots:
	virtual void closePressed();
	virtual void updatePressed();
};
#endif // LOGDLG_H
