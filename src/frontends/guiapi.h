// -*- C++ -*-

#include "LString.h"

class LyXView;
class Dialogs;
class InsetCommand;
class InsetError;
class InsetERT;
class InsetExternal;
class InsetFloat;
class InsetGraphics;
class InsetInclude;
class InsetInfo;
class InsetMinipage;
class InsetTabular;

extern "C" {

	void gui_ShowAboutlyx(LyXView &, Dialogs &);
	void gui_ShowBibitem(InsetCommand * ic, LyXView & lv, Dialogs & d);
	void gui_ShowBibtex(InsetCommand * ic, LyXView & lv, Dialogs & d);
	void gui_ShowCharacter(LyXView & lv, Dialogs & d);
	void gui_SetUserFreeFont(LyXView & lv, Dialogs & d);
	void gui_ShowCitation(InsetCommand * ic, LyXView & lv, Dialogs & d);
	void gui_CreateCitation(string const &, LyXView & lv, Dialogs & d);
	void gui_ShowDocument(LyXView & lv, Dialogs & d);
	void gui_ShowError(InsetError * ie, LyXView & lv, Dialogs & d);
	void gui_ShowERT(InsetERT * ie, LyXView & lv, Dialogs & d);
	void gui_UpdateERT(InsetERT * ie, LyXView & lv, Dialogs & d);
	void gui_ShowExternal(InsetExternal * ie, LyXView & lv, Dialogs & d);
	void gui_ShowFile(string const & f, LyXView & lv, Dialogs & d);
	void gui_ShowFloat(InsetFloat *, LyXView & lv, Dialogs &);
	void gui_ShowForks(LyXView & lv, Dialogs & d);
	void gui_ShowGraphics(InsetGraphics *, LyXView & lv, Dialogs & d);
	void gui_ShowInclude(InsetInclude *, LyXView & lv, Dialogs & d);
	void gui_ShowIndex(InsetCommand *, LyXView & lv, Dialogs & d);
	void gui_CreateIndex(string const &, LyXView & lv, Dialogs & d);
	void gui_ShowInfo(InsetInfo *, LyXView & lv, Dialogs & d);
	void gui_ShowLogFile(LyXView & lv, Dialogs & d);
	void gui_ShowMathPanel(LyXView & lv, Dialogs & d);
	void gui_ShowMinipage(InsetMinipage *, LyXView & lv, Dialogs & d);
	void gui_UpdateMinipage(InsetMinipage *, LyXView & lv, Dialogs & d);
	void gui_ShowParagraph(LyXView & lv, Dialogs & d);
	void gui_UpdateParagraph(LyXView & lv, Dialogs & d);
	void gui_ShowPreamble(LyXView & lv, Dialogs & d);
	void gui_ShowPreferences(LyXView & lv, Dialogs & d);
	void gui_ShowPrint(LyXView & lv, Dialogs & d);
	void gui_ShowRef(InsetCommand *, LyXView &, Dialogs &);
	void gui_CreateRef(string const &, LyXView &, Dialogs &);
	void gui_ShowSearch(LyXView & lv, Dialogs & d);
	void gui_ShowSendto(LyXView & lv, Dialogs & d);
	void gui_ShowSpellchecker(LyXView & lv, Dialogs & d);
	void gui_ShowSpellcheckerPreferences(LyXView & lv, Dialogs & d);
	void gui_ShowTabular(InsetTabular *, LyXView &, Dialogs &);
	void gui_UpdateTabular(InsetTabular *, LyXView &, Dialogs &);
	void gui_ShowTabularCreate(LyXView & lv, Dialogs & d);
	void gui_ShowTexinfo(LyXView & lv, Dialogs & d);
	void gui_ShowThesaurus(string const &, LyXView &, Dialogs &);
	void gui_ShowTOC(InsetCommand *, LyXView &, Dialogs &);
	void gui_CreateTOC(string const &, LyXView &, Dialogs &);
	void gui_ShowUrl(InsetCommand *, LyXView &, Dialogs &);
	void gui_CreateUrl(string const &, LyXView &, Dialogs &);
	void gui_ShowVCLogFile(LyXView & lv, Dialogs & d);

} // extern "C"
