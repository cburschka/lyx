/**
 * \file guiapi.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "guiapi.h"
#include "Dialogs.h"

extern "C" {

void gui_show_dialog(Dialogs * d, char const * name, char const * data)
{
	d->show(name, data, 0);
}
		
void gui_ShowCharacter(Dialogs & d)
{
	d.showCharacter();
}


void gui_SetUserFreeFont(Dialogs & d)
{
	d.setUserFreeFont();
}


void gui_ShowDocument(Dialogs & d)
{
	d.showDocument();
}


void gui_ShowFile(string const & f, Dialogs & d)
{
	d.showFile(f);
}


void gui_ShowForks(Dialogs & d)
{
	d.showForks();
}


void gui_ShowLogFile(Dialogs & d)
{
	d.showLogFile();
}


void gui_ShowMathPanel(Dialogs & d)
{
	d.showMathPanel();
}


void gui_ShowParagraph(Dialogs & d)
{
	d.showParagraph();
}


void gui_UpdateParagraph(Dialogs & d)
{
	d.updateParagraph();
}


void gui_ShowPreamble(Dialogs & d)
{
	d.showPreamble();
}


void gui_ShowPreferences(Dialogs & d)
{
	d.showPreferences();
}


void gui_ShowPrint(Dialogs & d)
{
	d.showPrint();
}


void gui_ShowSearch(Dialogs & d)
{
	d.showSearch();
}


void gui_ShowSendto(Dialogs & d)
{
	d.showSendto();
}


void gui_ShowSpellchecker(Dialogs & d)
{
	d.showSpellchecker();
}


void gui_ShowTexinfo(Dialogs & d)
{
	d.showTexinfo();
}


void gui_ShowThesaurus(string const & s, Dialogs & d)
{
	d.showThesaurus(s);
}


void gui_ShowVCLogFile(Dialogs & d)
{
	d.showVCLogFile();
}

} // extern "C"
