// -*- C++ -*-
/**
 * \file GUI.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef GUI_H
#define GUI_H

#include "ButtonController.h"
#include "ViewBase.h"

/** This class instantiates and makes available the GUI-specific
    ButtonController and View.
 */
template <class Controller, class GUIview, class Policy, class GUIbc>
class GUI : public Controller {
public:
	///
	GUI(LyXView & lv, Dialogs & d) : Controller(lv, d), view_(*this, d) {}
	///
	virtual ButtonControllerBase & bc() { return bc_; }
	///
	virtual ViewBase & view() { return view_; }

	friend void gui_ShowAboutlyx(LyXView &, Dialogs &);
	friend void gui_ShowBibitem(InsetCommand *, LyXView &, Dialogs &);
	friend void gui_ShowBibtex(InsetCommand *, LyXView &, Dialogs &);
	friend void gui_ShowCharacter(LyXView &, Dialogs &);
	friend void gui_SetUserFreeFont(LyXView &, Dialogs &);
	friend void gui_ShowCitation(InsetCommand *, LyXView &, Dialogs &);
	friend void gui_CreateCitation(string const &, LyXView &, Dialogs &);
	friend void gui_ShowDocument(LyXView &, Dialogs &);
	friend void gui_ShowError(InsetError *, LyXView &, Dialogs &);
	friend void gui_ShowERT(InsetERT *, LyXView &, Dialogs &);
	friend void gui_UpdateERT(InsetERT *, LyXView &, Dialogs &);
	friend void gui_ShowExternal(InsetExternal *, LyXView &, Dialogs &);
	friend void gui_ShowFloat(InsetFloat *, LyXView &, Dialogs &);
	friend void gui_ShowForks(LyXView &, Dialogs &);
	friend void gui_ShowGraphics(InsetGraphics *, LyXView &, Dialogs &);
	friend void gui_ShowInclude(InsetInclude *, LyXView &, Dialogs &);
	friend void gui_ShowIndex(InsetCommand *, LyXView &, Dialogs &);
	friend void gui_CreateIndex(LyXView &, Dialogs &);
	friend void gui_ShowLogFile(LyXView &, Dialogs &);
	friend void gui_ShowMathPanel(LyXView &, Dialogs &);
	friend void gui_ShowMinipage(InsetMinipage *, LyXView &, Dialogs &);
	friend void gui_UpdateMinipage(InsetMinipage *, LyXView &, Dialogs &);
	friend void gui_ShowParagraph(LyXView &, Dialogs &);
	friend void gui_ShowPreamble(LyXView &, Dialogs &);
	friend void gui_ShowPreferences(LyXView &, Dialogs &);
	friend void gui_ShowPrint(LyXView &, Dialogs &);
	friend void gui_ShowRef(InsetCommand *, LyXView &, Dialogs &);
	friend void gui_CreateRef(string const &, LyXView &, Dialogs &);
	friend void gui_ShowSearch(LyXView &, Dialogs &);
	friend void gui_ShowSendto(LyXView &, Dialogs &);
	friend void gui_ShowSpellchecker(LyXView &, Dialogs &);
	friend void gui_ShowTabularCreate(LyXView &, Dialogs &);
	friend void gui_ShowTexinfo(LyXView &, Dialogs &);
	friend void gui_ShowTOC(InsetCommand *, LyXView &, Dialogs &);
	friend void gui_CreateTOC(string const &, LyXView &, Dialogs &);
	friend void gui_ShowUrl(InsetCommand *, LyXView &, Dialogs &);
	friend void gui_CreateUrl(string const &, LyXView &, Dialogs &);
	friend void gui_ShowVCLogFile(LyXView &, Dialogs &);

private:
	///
	ButtonController<Policy, GUIbc> bc_;
	///
	GUIview view_;
};

#endif // GUI_H
