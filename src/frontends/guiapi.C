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

#ifdef __GNUG__
#pragma implementation
#endif

#include "guiapi.h"
#include "Dialogs.h"

extern "C" {

	void gui_ShowAboutlyx(Dialogs & d)
	{
		d.showAboutlyx();
	}


	void gui_ShowBibitem(InsetCommand * ic, Dialogs & d)
	{
		d.showBibitem(ic);
	}


	void gui_ShowBibtex(InsetCommand * ic, Dialogs & d)
	{
		d.showBibtex(ic);
	}


	void gui_ShowCharacter(Dialogs & d)
	{
		d.showCharacter();
	}


	void gui_SetUserFreeFont(Dialogs & d)
	{
		d.setUserFreeFont();
	}


	void gui_ShowCitation(InsetCommand * ic, Dialogs & d)
	{
		d.showCitation(ic);
	}


	void gui_CreateCitation(string const & s, Dialogs & d)
	{
		d.createCitation(s);
	}


	void gui_ShowDocument(Dialogs & d)
	{
		d.showDocument();
	}


	void gui_ShowError(InsetError * ie, Dialogs & d)
	{
		d.showError(ie);
	}


	void gui_ShowERT(InsetERT * ie, Dialogs & d)
	{
		d.showERT(ie);
	}


	void gui_UpdateERT(InsetERT * ie, Dialogs & d)
	{
		d.updateERT(ie);
	}


	void gui_ShowExternal(InsetExternal * ie, Dialogs & d)
	{
		d.showExternal(ie);
	}


	void gui_ShowFile(string const & f, Dialogs & d)
	{
		d.showFile(f);
	}


	void gui_ShowFloat(InsetFloat * ifl, Dialogs & d)
	{
		d.showFloat(ifl);
	}


	void gui_ShowForks(Dialogs & d)
	{
		d.showForks();
	}


	void gui_ShowGraphics(InsetGraphics * ig, Dialogs & d)
	{
		d.showGraphics(ig);
	}


	void gui_ShowInclude(InsetInclude * ii, Dialogs & d)
	{
		d.showInclude(ii);
	}


	void gui_ShowIndex(InsetCommand * ic, Dialogs & d)
	{
		d.showIndex(ic);
	}


	void gui_CreateIndex(Dialogs & d)
	{
		d.createIndex();
	}


	void gui_ShowLogFile(Dialogs & d)
	{
		d.showLogFile();
	}


	void gui_ShowMathPanel(Dialogs & d)
	{
		d.showMathPanel();
	}


	void gui_ShowMinipage(InsetMinipage * im, Dialogs & d)
	{
		d.showMinipage(im);
	}


	void gui_UpdateMinipage(InsetMinipage * im, Dialogs & d)
	{
		d.updateMinipage(im);
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


	void gui_ShowRef(InsetCommand * ic, Dialogs & d)
	{
		d.showRef(ic);
	}


	void gui_CreateRef(string const & s, Dialogs & d)
	{
		d.createRef(s);
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


	void gui_ShowTabular(InsetTabular * it, Dialogs & d)
	{
		d.showTabular(it);
	}


	void gui_UpdateTabular(InsetTabular * it, Dialogs & d)
	{
		d.updateTabular(it);
	}


	void gui_ShowTabularCreate(Dialogs & d)
	{
		d.showTabularCreate();
	}


	void gui_ShowTexinfo(Dialogs & d)
	{
		d.showTexinfo();
	}


	void gui_ShowThesaurus(string const & s, Dialogs & d)
	{
		d.showThesaurus(s);
	}


	void gui_ShowTOC(InsetCommand * ic, Dialogs & d)
	{
		d.showTOC(ic);
	}


	void gui_CreateTOC(string const & s, Dialogs & d)
	{
		d.createTOC(s);
	}


	void gui_ShowUrl(InsetCommand * ic, Dialogs & d)
	{
		d.showUrl(ic);
	}


	void gui_CreateUrl(string const & s, Dialogs & d)
	{
		d.createUrl(s);
	}


	void gui_ShowVCLogFile(Dialogs & d)
	{
		d.showVCLogFile();
	}

} // extern "C"
