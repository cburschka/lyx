/*
 * parageneraldlg.h
 * (C) 2000 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */
 
#ifndef ParaGeneralDialog_included
#define ParaGeneralDialog_included

#include "dlg/parageneraldlgdata.h"
#include "dlg/paraabovedlgdata.h" 
#include "dlg/parabelowdlgdata.h"

class ParaDialog;

class ParaGeneralDialog : public ParaGeneralDialogData
{
    Q_OBJECT

public:

	ParaGeneralDialog (QWidget* parent = NULL, const char* name = NULL);

	virtual ~ParaGeneralDialog();

	friend class ParaDialog;
     
protected slots:

	void spaceaboveHighlighted(int);
	void spacebelowHighlighted(int);

private:
	ParaAboveDialogData *abovepage;
	ParaBelowDialogData *belowpage;
};

#endif // ParaGeneralDialog_included
