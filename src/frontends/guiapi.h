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

extern "C" {

void gui_show_dialog(Dialogs *, char const * name, char const * data);

void gui_ShowDocument(Dialogs &);
void gui_ShowForks(Dialogs &);
void gui_ShowMathPanel(Dialogs &);
void gui_ShowPreamble(Dialogs &);
void gui_ShowPreferences(Dialogs &);
void gui_ShowPrint(Dialogs &);
void gui_ShowSearch(Dialogs &);
void gui_ShowSendto(Dialogs &);
void gui_ShowSpellchecker(Dialogs &);
void gui_ShowTexinfo(Dialogs &);

} // extern "C"

#endif // GUIAPI_H
