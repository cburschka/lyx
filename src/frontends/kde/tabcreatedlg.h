/*
 * tabcreatedialog.h
 * (C) 2000 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

#ifndef TABCREATEDIALOG_H
#define TABCREATEDIALOG_H

#include "dlg/tabcreatedlgdata.h"
#include "FormTabularCreate.h" 

class TabularCreateDialog : public TabularCreateDialogData
{
    Q_OBJECT
public:
	TabularCreateDialog (FormTabularCreate *form, QWidget* parent = 0, const char *name = 0);
	virtual ~TabularCreateDialog();

protected slots:
	virtual void clickedInsert();
	virtual void clickedCancel();
	virtual void colsChanged(unsigned int);
	virtual void rowsChanged(unsigned int);

private:
	FormTabularCreate *form_;
};
#endif
