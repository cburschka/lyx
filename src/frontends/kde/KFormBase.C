/**
 * \file KDEFormBase.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"
#include "KFormBase.h"
#include "ControlButtons.h" 

#include <qdialog.h>
 
KFormDialogBase::KFormDialogBase(ControlButtons & c)
	: ViewBC<kdeBC>(c)
{
}

 
void KFormDialogBase::show()
{
	if (!dialog())
		build();
 
	bc().refresh();

	update();

	dialog()->raise();
	dialog()->setActiveWindow();
	dialog()->show();
}

	
void KFormDialogBase::hide()
{
	if (dialog()->isVisible())
		dialog()->hide();
}
