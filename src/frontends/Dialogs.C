/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ======================================================
 *
 * \file Dialogs.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * Methods common to all frontends' Dialogs that should not be inline
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"

#include "support/LAssert.h"

#include "guiapi.h"

// Signal enabling all visible dialogs to be redrawn if so desired.
// E.g., when the GUI colours have been remapped.
boost::signal0<void> Dialogs::redrawGUI;

extern LyXView * dialogs_lyxview;


// toggle tooltips on/off in all dialogs.
boost::signal0<void> Dialogs::toggleTooltips;

void Dialogs::showAboutlyx()
{
	gui_ShowAboutlyx(*dialogs_lyxview, *this);
}


void Dialogs::showBibitem(InsetCommand * ic)
{
	gui_ShowBibitem(ic, *dialogs_lyxview, *this);
}


void Dialogs::showBibtex(InsetCommand * ic)
{
	gui_ShowBibtex(ic, *dialogs_lyxview, *this);
}


void Dialogs::showCharacter()
{
	gui_ShowCharacter(*dialogs_lyxview, *this);
}


void Dialogs::setUserFreeFont()
{
	gui_SetUserFreeFont(*dialogs_lyxview, *this);
}


void Dialogs::showCitation(InsetCommand * ic)
{
	gui_ShowCitation(ic, *dialogs_lyxview, *this);
}


void Dialogs::createCitation(string const & s)
{
	gui_CreateCitation(s, *dialogs_lyxview, *this);
}


void Dialogs::showDocument()
{
	gui_ShowDocument(*dialogs_lyxview, *this);
}


void Dialogs::showError(InsetError * ie)
{
	gui_ShowError(ie, *dialogs_lyxview, *this);
}


void Dialogs::showERT(InsetERT * ie)
{
	gui_ShowERT(ie, *dialogs_lyxview, *this);
}


void Dialogs::updateERT(InsetERT * ie)
{
	gui_UpdateERT(ie, *dialogs_lyxview, *this);
}


void Dialogs::showExternal(InsetExternal * ie)
{
	gui_ShowExternal(ie, *dialogs_lyxview, *this);
}


void Dialogs::showFile(string const & f)
{
	gui_ShowFile(f, *dialogs_lyxview, *this);
}


void Dialogs::showFloat(InsetFloat * ifl)
{
	gui_ShowFloat(ifl, *dialogs_lyxview, *this);
}


void Dialogs::showForks()
{
	gui_ShowForks(*dialogs_lyxview, *this);
}


void Dialogs::showGraphics(InsetGraphics * ig)
{
	gui_ShowGraphics(ig, *dialogs_lyxview, *this);
}


void Dialogs::showInclude(InsetInclude * ii)
{
	gui_ShowInclude(ii, *dialogs_lyxview, *this);
}


void Dialogs::showIndex(InsetCommand * ic)
{
	gui_ShowIndex(ic, *dialogs_lyxview, *this);
}


void Dialogs::createIndex(string const & s)
{
	gui_CreateIndex(s, *dialogs_lyxview, *this);
}


void Dialogs::showInfo(InsetInfo * /*ii*/)
{
#if 0
	gui_ShowInfo(ii, *dialogs_lyxview, *this);
#endif
}


void Dialogs::showLogFile()
{
	gui_ShowLogFile(*dialogs_lyxview, *this);
}


void Dialogs::showMathPanel()
{
	gui_ShowMathPanel(*dialogs_lyxview, *this);
}


void Dialogs::showMinipage(InsetMinipage * im)
{
	gui_ShowMinipage(im, *dialogs_lyxview, *this);
}


void Dialogs::updateMinipage(InsetMinipage * im)
{
	gui_UpdateMinipage(im, *dialogs_lyxview, *this);
}


void Dialogs::showParagraph()
{
	gui_ShowParagraph(*dialogs_lyxview, *this);
}

void Dialogs::updateParagraph()
{
#if 0
	gui_UpdateParagraph(*dialogs_lyxview, *this);
#endif
}


void Dialogs::showPreamble()
{
	gui_ShowPreamble(*dialogs_lyxview, *this);
}


void Dialogs::showPreferences()
{
	gui_ShowPreferences(*dialogs_lyxview, *this);
}


void Dialogs::showPrint()
{
	gui_ShowPrint(*dialogs_lyxview, *this);
}


void Dialogs::showRef(InsetCommand * ic)
{
	gui_ShowRef(ic, *dialogs_lyxview, *this);
}


void Dialogs::createRef(string const & s)
{
	gui_CreateRef(s, *dialogs_lyxview, *this);
}


void Dialogs::showSearch()
{
	gui_ShowSearch(*dialogs_lyxview, *this);
}


void Dialogs::showSendto()
{
	gui_ShowSendto(*dialogs_lyxview, *this);
}


void Dialogs::showSpellchecker()
{
	gui_ShowSpellchecker(*dialogs_lyxview, *this);
}


void Dialogs::showSpellcheckerPreferences()
{
	gui_ShowSpellcheckerPreferences(*dialogs_lyxview, *this);
}


void Dialogs::showTabular(InsetTabular * it)
{
	gui_ShowTabular(it, *dialogs_lyxview, *this);
}


void Dialogs::updateTabular(InsetTabular * it)
{
	gui_UpdateTabular(it, *dialogs_lyxview, *this);
}


void Dialogs::showTabularCreate()
{
	gui_ShowTabularCreate(*dialogs_lyxview, *this);
}


void Dialogs::showThesaurus(string const & s)
{
	gui_ShowThesaurus(s, *dialogs_lyxview, *this);
}


void Dialogs::showTexinfo()
{
	gui_ShowTexinfo(*dialogs_lyxview, *this);
}


void Dialogs::showTOC(InsetCommand * ic)
{
	gui_ShowTOC(ic, *dialogs_lyxview, *this);
}


void Dialogs::createTOC(string const & s)
{
	gui_CreateTOC(s, *dialogs_lyxview, *this);
}


void Dialogs::showUrl(InsetCommand * ic)
{
	gui_ShowUrl(ic, *dialogs_lyxview, *this);
}


void Dialogs::createUrl(string const & s)
{
	gui_CreateUrl(s, *dialogs_lyxview, *this);
}


void Dialogs::showVCLogFile()
{
	gui_ShowVCLogFile(*dialogs_lyxview, *this);
}


//void Dialogs::add(DialogBase * ptr)
//{
//	lyx::Assert(ptr);
//	dialogs_.push_back(db_ptr(ptr));
//}
