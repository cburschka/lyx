/**
 * \file tabcreatedlg.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef TABCREATEDIALOG_H
#define TABCREATEDIALOG_H

#include "FormTabularCreate.h" 
#include "dlg/tabcreatedlgdata.h"

class TabularCreateDialog : public TabularCreateDialogData
{
	Q_OBJECT
public:
	TabularCreateDialog (FormTabularCreate * form, QWidget * parent = 0, char const * name = 0);
	virtual ~TabularCreateDialog();

protected slots:
	virtual void clickedInsert();
	virtual void clickedCancel();
	virtual void colsChanged(unsigned int);
	virtual void rowsChanged(unsigned int);
	virtual void closeEvent(QCloseEvent * e); 

private:
	FormTabularCreate * form_;
};
#endif // TABCREATEDIALOG_H
