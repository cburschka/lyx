/**
 * \file vclogdlg.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef VCLOGDLG_H
#define VCLOGDLG_H

#include <config.h>
#include "support/lstrings.h"
 
#include "dlg/logdlgdata.h"

class FormVCLog;
 
class VCLogDialog : public LogDialogData
{
	Q_OBJECT

public:
	VCLogDialog(FormVCLog * f, QWidget * parent = NULL, char const * name = NULL);

	void setLogText(string const & text);
 
protected slots:
	virtual void closePressed();
 
	virtual void updatePressed();
 
	virtual void closeEvent(QCloseEvent * e);
 
private:
	FormVCLog * form_;
};
#endif // LOGVCDLG_H
