/**
 * \file gnome/Dialogs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif


#include "Dialogs.h"

#include "GUI.h"
#include "gnomeBC.h"

#include "frontends/LyXView.h"

#include "GAbout.h"
#include "GError.h"
#include "GERT.h"
#include "GFloat.h"
#include "GLog.h"
#include "GPreamble.h"
#include "GTabularCreate.h"
#include "GUrl.h"

#include "Tooltips.h"

bool Dialogs::tooltipsEnabled()
{
	return Tooltips::enabled();
}
Dialogs::Dialogs(LyXView * lv)
{

	add(new GUI<ControlError,GErrorDialog, 
	    OkCancelPolicy, gnomeBC>(*lv, *this));
	add(new GUI<ControlERT, GERT,
	    NoRepeatedApplyReadOnlyPolicy, gnomeBC>(*lv, *this));
	add(new GUI<ControlUrl, GUrl,
	    NoRepeatedApplyReadOnlyPolicy, gnomeBC>(*lv,*this));
	add(new GUI<ControlPreamble, GPreamble,
	    NoRepeatedApplyReadOnlyPolicy, gnomeBC>(*lv, *this));
	add(new GUI<ControlTabularCreate, GTabularCreate,
	    OkApplyCancelReadOnlyPolicy, gnomeBC>(*lv, *this));
	add(new GUI<ControlLog, GLog,
	    OkCancelPolicy, gnomeBC>(*lv, *this));
	add(new GUI<ControlAboutlyx, GAbout,
	    OkCancelPolicy, gnomeBC>(*lv, *this));
	add(new GUI<ControlFloat, GFloat,
	    NoRepeatedApplyReadOnlyPolicy, gnomeBC>(*lv, *this));
	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAll.connect(hideBufferDependent);
}
