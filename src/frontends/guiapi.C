/**
 * \file guiapi.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "guiapi.h"
#include "Dialogs.h"


extern "C" {

void gui_show_dialog(Dialogs * d, char const * name, char const * data)
{
	d->show(name, data, 0);
}

void gui_ShowPreamble(Dialogs & d)
{
	d.showPreamble();
}


void gui_ShowPreferences(Dialogs & d)
{
	d.showPreferences();
}


void gui_ShowSpellchecker(Dialogs & d)
{
	d.showSpellchecker();
}


} // extern "C"
