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

class FormLog;
 
class LogDialog : public LogDialogData
{
	Q_OBJECT

public:
	LogDialog(FormLog * f, QWidget * parent = NULL, char const * name = NULL);

	void setLogText(string const & text);
 
protected slots:
	virtual void closePressed();
	virtual void updatePressed();
	virtual void closeEvent(QCloseEvent * e);
 
private:
	FormLog * form_;
};
#endif // LOGDLG_H
