// -*- C++ -*-
/**
 * \file guiapi.h
 * See the file COPYING.
 *
 * \author Lars Gullik Bjønnes 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef GUIAPI_H
#define GUIAPI_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

class Dialogs;
class InsetCommand;
class InsetError;
class InsetERT;
class InsetExternal;
class InsetFloat;
class InsetGraphics;
class InsetInclude;
class InsetMinipage;
class InsetTabular;

extern "C" {

	void gui_ShowAboutlyx(Dialogs &);
	void gui_ShowBibitem(InsetCommand *, Dialogs &);
	void gui_ShowBibtex(InsetCommand *, Dialogs &);
	void gui_ShowCharacter(Dialogs &);
	void gui_SetUserFreeFont(Dialogs &);
	void gui_ShowCitation(InsetCommand *, Dialogs &);
	void gui_CreateCitation(string const &, Dialogs &);
	void gui_ShowDocument(Dialogs &);
	void gui_ShowError(InsetError *, Dialogs &);
	void gui_ShowERT(InsetERT *, Dialogs &);
	void gui_UpdateERT(InsetERT *, Dialogs &);
	void gui_ShowExternal(InsetExternal *, Dialogs &);
	void gui_ShowFile(string const &, Dialogs &);
	void gui_ShowFloat(InsetFloat *, Dialogs &);
	void gui_ShowForks(Dialogs &);
	void gui_ShowGraphics(InsetGraphics *, Dialogs &);
	void gui_ShowInclude(InsetInclude *, Dialogs &);
	void gui_ShowIndex(InsetCommand *, Dialogs &);
	void gui_CreateIndex(Dialogs &);
	void gui_ShowLogFile(Dialogs &);
	void gui_ShowMathPanel(Dialogs &);
	void gui_ShowMinipage(InsetMinipage *, Dialogs &);
	void gui_UpdateMinipage(InsetMinipage *, Dialogs &);
	void gui_ShowParagraph(Dialogs &);
	void gui_UpdateParagraph(Dialogs &);
	void gui_ShowPreamble(Dialogs &);
	void gui_ShowPreferences(Dialogs &);
	void gui_ShowPrint(Dialogs &);
	void gui_ShowRef(InsetCommand *, Dialogs &);
	void gui_CreateRef(string const &, Dialogs &);
	void gui_ShowSearch(Dialogs &);
	void gui_ShowSendto(Dialogs &);
	void gui_ShowSpellchecker(Dialogs &);
	void gui_ShowTabular(InsetTabular *, Dialogs &);
	void gui_UpdateTabular(InsetTabular *, Dialogs &);
	void gui_ShowTabularCreate(Dialogs &);
	void gui_ShowTexinfo(Dialogs &);
	void gui_ShowThesaurus(string const &, Dialogs &);
	void gui_ShowTOC(InsetCommand *, Dialogs &);
	void gui_CreateTOC(string const &, Dialogs &);
	void gui_ShowUrl(InsetCommand *, Dialogs &);
	void gui_CreateUrl(string const &, Dialogs &);
	void gui_ShowVCLogFile(Dialogs &);

} // extern "C"

#endif // GUIAPI_H
