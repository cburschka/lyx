// -*- C++ -*-
/**
 * \file guiapi.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef GUIAPI_H
#define GUIAPI_H


#include "LString.h"

class Dialogs;
class InsetFloat;
class InsetWrap;
class InsetGraphics;
class InsetInclude;
class InsetMinipage;
class InsetTabular;

extern "C" {

void gui_show_dialog(Dialogs *, char const * name, char const * data);

void gui_ShowAboutlyx(Dialogs &);
void gui_ShowCharacter(Dialogs &);
void gui_SetUserFreeFont(Dialogs &);
void gui_ShowDocument(Dialogs &);
void gui_ShowFile(string const &, Dialogs &);
void gui_ShowFloat(InsetFloat *, Dialogs &);
void gui_ShowForks(Dialogs &);
void gui_ShowGraphics(InsetGraphics *, Dialogs &);
void gui_ShowInclude(InsetInclude *, Dialogs &);
void gui_ShowLogFile(Dialogs &);
void gui_ShowMathPanel(Dialogs &);
void gui_ShowMinipage(InsetMinipage *, Dialogs &);
void gui_UpdateMinipage(InsetMinipage *, Dialogs &);
void gui_ShowParagraph(Dialogs &);
void gui_UpdateParagraph(Dialogs &);
void gui_ShowPreamble(Dialogs &);
void gui_ShowPreferences(Dialogs &);
void gui_ShowPrint(Dialogs &);
void gui_ShowSearch(Dialogs &);
void gui_ShowSendto(Dialogs &);
void gui_ShowSpellchecker(Dialogs &);
void gui_ShowTabular(InsetTabular *, Dialogs &);
void gui_UpdateTabular(InsetTabular *, Dialogs &);
void gui_ShowTabularCreate(Dialogs &);
void gui_ShowTexinfo(Dialogs &);
void gui_ShowThesaurus(string const &, Dialogs &);
void gui_ShowVCLogFile(Dialogs &);
void gui_ShowWrap(InsetWrap *, Dialogs &);

} // extern "C"

#endif // GUIAPI_H
