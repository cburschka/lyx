/**
 * \file parageneraldlg.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef PARAGENERALDIALOG_H
#define PARAGENERALDIALOG_H

#include "dlg/parageneraldlgdata.h"
#include "dlg/paraabovedlgdata.h" 
#include "dlg/parabelowdlgdata.h"

class ParaDialog;

class ParaGeneralDialog : public ParaGeneralDialogData
{
	Q_OBJECT

public:

	ParaGeneralDialog(QWidget * parent = NULL, char const * name = NULL);

	virtual ~ParaGeneralDialog();

	friend class ParaDialog;
     
protected slots:

	void spaceaboveHighlighted(int);
	void spacebelowHighlighted(int);

private:
	ParaAboveDialogData * abovepage;
	ParaBelowDialogData * belowpage;
};

#endif // PARAGENERALDIALOG_H
