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

class ParaAboveDialog : public ParaAboveDialogData
{
    Q_OBJECT
     
public:
	ParaAboveDialog(QWidget *p, const char *name) : ParaAboveDialogData(p,name) {};
	~ParaAboveDialog() {};
	
	friend class ParaGeneralDialog;
	friend class ParaDialog; 
};
 
class ParaBelowDialog : public ParaBelowDialogData
{
    Q_OBJECT
     
public:
	ParaBelowDialog(QWidget *p, const char *name) : ParaBelowDialogData(p,name) {};
	~ParaBelowDialog() {};
	
	friend class ParaGeneralDialog;
	friend class ParaDialog; 
};
 
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
	void createUnits(QComboBox *box);
	ParaAboveDialog *abovepage;
	ParaBelowDialog *belowpage;
};

#endif // ParaGeneralDialog_included
