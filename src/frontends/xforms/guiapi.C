#include <config.h>

#include "guiapi.h"

#include "FormAboutlyx.h"
#include "forms/form_aboutlyx.h"

#include "FormBibitem.h"
#include "forms/form_bibitem.h"

#include "FormBibtex.h"
#include "forms/form_bibtex.h"

#include "FormBrowser.h"
#include "forms/form_browser.h"

#include "FormCharacter.h"
#include "forms/form_character.h"

#include "FormCitation.h"
#include "forms/form_citation.h"

#include "FormDocument.h"
#include "forms/form_document.h"

#include "FormError.h"
#include "forms/form_error.h"

#include "FormERT.h"
#include "forms/form_ert.h"

#include "FormExternal.h"
#include "forms/form_external.h"

#include "FormFloat.h"
#include "forms/form_float.h"

#include "FormForks.h"
#include "forms/form_forks.h"

#include "FormGraphics.h"
#include "forms/form_graphics.h"

#include "FormInclude.h"
#include "forms/form_include.h"

#include "FormIndex.h"
#include "forms/form_index.h"

#include "FormLog.h"

#include "FormMathsPanel.h"
#include "FormMathsBitmap.h"
#include "FormMathsDeco.h"
#include "FormMathsDelim.h"
#include "FormMathsMatrix.h"
#include "FormMathsSpace.h"
#include "FormMathsStyle.h"

#include "forms/form_maths_deco.h"
#include "forms/form_maths_delim.h"
#include "forms/form_maths_matrix.h"
#include "forms/form_maths_panel.h"
#include "forms/form_maths_space.h"
#include "forms/form_maths_style.h"

#include "FormMinipage.h"
#include "forms/form_minipage.h"

#include "FormParagraph.h"
#include "forms/form_paragraph.h"

#include "FormPreamble.h"
#include "forms/form_preamble.h"

#include "FormPreferences.h"
#include "forms/form_preferences.h"

#include "FormPrint.h"
#include "forms/form_print.h"

#include "FormRef.h"
#include "forms/form_ref.h"

#include "FormSearch.h"
#include "forms/form_search.h"

#include "FormSendto.h"
#include "forms/form_sendto.h"

#include "FormShowFile.h"

#include "FormSpellchecker.h"
#include "forms/form_spellchecker.h"

#include "FormTabular.h"
#include "forms/form_tabular.h"

#include "FormTabularCreate.h"
#include "forms/form_tabular_create.h"

#include "FormTexinfo.h"
#include "forms/form_texinfo.h"

#ifdef HAVE_LIBAIKSAURUS
#include "FormThesaurus.h"
#include "forms/form_thesaurus.h"
#endif
 
#include "FormToc.h"
#include "forms/form_toc.h"

#include "FormUrl.h"
#include "forms/form_url.h"

#include "FormVCLog.h"

#include "xformsBC.h"
#include "combox.h"

#include "controllers/GUI.h"
#include "insets/inseterror.h"

#include "LyXView.h"

#include "ControlAboutlyx.h"
#include "ControlBibitem.h"
#include "ControlBibtex.h"
#include "ControlCharacter.h"
#include "ControlCitation.h"
#include "ControlError.h"
#include "ControlERT.h"
#include "ControlExternal.h"
#include "ControlFloat.h"
#include "ControlForks.h"
#include "ControlGraphics.h"
#include "insets/insetgraphicsParams.h"
#include "ControlInclude.h"
#include "ControlIndex.h"
#include "ControlLog.h"
#include "ControlMinipage.h"
#include "ControlParagraph.h"
#include "ControlPreamble.h"
#include "ControlPrint.h"
#include "ControlRef.h"
#include "ControlSearch.h"
#include "ControlSendto.h"
#include "ControlShowFile.h"
#include "ControlSpellchecker.h"
#include "ControlTabularCreate.h"
#include "ControlTexinfo.h"
#include "ControlThesaurus.h"
#include "ControlToc.h"
#include "ControlUrl.h"
#include "ControlVCLog.h"


namespace { // anon

// I belive that this one is buggy (Lgb)
// But not if used correctly. (Lgb)
template <class T>
class GUISingleton {
public:
	static
	T & get(LyXView & lv, Dialogs & d) {
		static T t(lv, d);
		return t;
	}
};


GUISingleton<GUI<ControlCharacter, FormCharacter,
		 OkApplyCancelReadOnlyPolicy, xformsBC> >
controlCharacterSingleton;

GUISingleton<GUI<ControlCitation, FormCitation,
		 NoRepeatedApplyReadOnlyPolicy, xformsBC> >
controlCitationSingleton;


GUISingleton<GUI<ControlERT, FormERT,
		 NoRepeatedApplyReadOnlyPolicy, xformsBC> >
controlERTSingleton;

GUISingleton<GUI<ControlIndex, FormIndex,
		 NoRepeatedApplyReadOnlyPolicy, xformsBC> >
controlIndexSingleton;

GUISingleton<GUI<ControlMinipage, FormMinipage,
		 NoRepeatedApplyReadOnlyPolicy, xformsBC> >
controlMinipageSingleton;

GUISingleton<FormPreferences> formPreferencesSingleton;

GUISingleton<GUI<ControlRef, FormRef,
		 NoRepeatedApplyReadOnlyPolicy, xformsBC> >
controlRefSingleton;

GUISingleton<FormTabular> formTabularSingleton;

GUISingleton<GUI<ControlToc, FormToc,
		 OkCancelPolicy, xformsBC> >
controlTocSingleton;

GUISingleton<GUI<ControlUrl, FormUrl,
		 NoRepeatedApplyReadOnlyPolicy, xformsBC> >
controlUrlSingleton;

} // anon


extern "C" {

	void gui_ShowAboutlyx(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlAboutlyx, FormAboutlyx,
			OkCancelPolicy, xformsBC> cal(lv, d);
		cal.show();
	}


	void gui_ShowBibitem(InsetCommand * ic, LyXView & lv, Dialogs & d)
	{
		static GUI<ControlBibitem, FormBibitem,
			OkCancelReadOnlyPolicy, xformsBC> cbi(lv, d);
		cbi.showInset(ic);
	}


	void gui_ShowBibtex(InsetCommand * ic, LyXView & lv, Dialogs & d)
	{
		static GUI<ControlBibtex, FormBibtex,
			OkCancelReadOnlyPolicy, xformsBC> cbt(lv, d);
		cbt.showInset(ic);
	}


	void gui_ShowCharacter(LyXView & lv, Dialogs & d)
	{
		controlCharacterSingleton.get(lv, d).show();
	}


	void gui_SetUserFreeFont(LyXView & lv, Dialogs & d)
	{
		controlCharacterSingleton.get(lv, d).apply();
	}


	void gui_ShowCitation(InsetCommand * ic, LyXView & lv, Dialogs & d)
	{
		controlCitationSingleton.get(lv, d).showInset(ic);
	}


	void gui_CreateCitation(string const & s, LyXView & lv, Dialogs & d)
	{
		controlCitationSingleton.get(lv, d).createInset(s);
	}


	void gui_ShowDocument(LyXView & lv, Dialogs & d)
	{
		static FormDocument fd(&lv, &d);
		fd.show();
	}


	void gui_ShowError(InsetError * ie, LyXView & lv, Dialogs & d)
	{
		static GUI<ControlError, FormError,
			OkCancelPolicy, xformsBC> ce(lv, d);
		ce.showInset(ie);
	}


	void gui_ShowERT(InsetERT * ie, LyXView & lv, Dialogs & d)
	{
		controlERTSingleton.get(lv, d).showInset(ie);
	}


	void gui_UpdateERT(InsetERT * ie, LyXView & lv, Dialogs & d)
	{
		controlERTSingleton.get(lv, d).showInset(ie);
	}


	void gui_ShowExternal(InsetExternal * ie, LyXView & lv, Dialogs & d)
	{
		static GUI<ControlExternal, FormExternal,
			OkApplyCancelReadOnlyPolicy, xformsBC> ce(lv, d);
		ce.showInset(ie);
	}


	void gui_ShowFile(string const & f, LyXView & lv, Dialogs & d)
	{
		static GUI<ControlShowFile, FormShowFile,
			OkCancelPolicy, xformsBC> csf(lv, d);
		csf.showFile(f);
	}


	void gui_ShowFloat(InsetFloat * ifl, LyXView & lv, Dialogs & d)
	{
		static GUI<ControlFloat, FormFloat,
			NoRepeatedApplyReadOnlyPolicy, xformsBC> cf(lv, d);
		cf.showInset(ifl);
	}


	void gui_ShowForks(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlForks, FormForks,
			OkApplyCancelPolicy, xformsBC> cf(lv, d);
		cf.show();
	}


	void gui_ShowGraphics(InsetGraphics * ig, LyXView & lv, Dialogs & d)
	{
		static GUI<ControlGraphics, FormGraphics,
			NoRepeatedApplyReadOnlyPolicy, xformsBC> cg(lv, d);
		cg.showInset(ig);
	}


	void gui_ShowInclude(InsetInclude * ii, LyXView & lv, Dialogs & d)
	{
		static GUI<ControlInclude, FormInclude,
			OkCancelReadOnlyPolicy, xformsBC> ci(lv, d);
		ci.showInset(ii);
	}


	void gui_ShowIndex(InsetCommand * ic, LyXView & lv, Dialogs & d)
	{
		controlIndexSingleton.get(lv, d).showInset(ic);
	}


	void gui_CreateIndex(LyXView & lv, Dialogs & d)
	{
		controlIndexSingleton.get(lv, d).createInset("");
	}


#if 0
	void gui_ShowInfo(InsetInfo *, LyXView & lv, Dialogs & d)
	{
	}
#endif


	void gui_ShowLogFile(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlLog, FormLog,
			OkCancelPolicy, xformsBC> cl(lv, d);
		cl.show();
	}


	void gui_ShowMathPanel(LyXView & lv, Dialogs & d)
	{
		static FormMathsPanel fmp(&lv, &d);
		fmp.show();
	}


	void gui_ShowMinipage(InsetMinipage * im, LyXView & lv, Dialogs & d)
	{
		controlMinipageSingleton.get(lv, d).showInset(im);
	}


	void gui_UpdateMinipage(InsetMinipage * im, LyXView & lv, Dialogs & d)
	{
		controlMinipageSingleton.get(lv, d).showInset(im);
	}


	void gui_ShowParagraph(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlParagraph, FormParagraph,
			OkApplyCancelReadOnlyPolicy, xformsBC> cp(lv, d);
		cp.show();
	}


#if 0
	void gui_UpdateParagraph(LyXView & lv, Dialogs & d)
	{
#warning FIXME!
		// This should be the same object as in gui_ShowParagraph
		static GUI<ControlParagraph, FormParagraph,
			OkApplyCancelReadOnlyPolicy, xformsBC> cp(lv, d);
#if 0
		cp.
#endif
	}
#endif


	void gui_ShowPreamble(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlPreamble, FormPreamble,
			NoRepeatedApplyReadOnlyPolicy, xformsBC> cp(lv, d);
		cp.show();
	}


	void gui_ShowPreferences(LyXView & lv, Dialogs & d)
	{
		formPreferencesSingleton.get(lv, d).show();
	}


	void gui_ShowPrint(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlPrint, FormPrint,
			OkApplyCancelPolicy, xformsBC> cp(lv, d);
		cp.show();
	}


	void gui_ShowRef(InsetCommand * ic, LyXView & lv, Dialogs & d)
	{
		controlRefSingleton.get(lv, d).showInset(ic);
	}


	void gui_CreateRef(string const & s, LyXView & lv, Dialogs & d)
	{
		controlRefSingleton.get(lv, d).createInset(s);
	}


	void gui_ShowSearch(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlSearch, FormSearch,
			NoRepeatedApplyReadOnlyPolicy, xformsBC> cs(lv, d);
		cs.show();
	}


	void gui_ShowSendto(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlSendto, FormSendto,
			OkApplyCancelPolicy, xformsBC> cs(lv, d);
		cs.show();
	}


	void gui_ShowSpellchecker(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlSpellchecker, FormSpellchecker,
			NoRepeatedApplyReadOnlyPolicy, xformsBC> cp(lv, d);
		cp.show();
	}


	void gui_ShowSpellcheckerPreferences(LyXView & lv, Dialogs & d)
	{
		formPreferencesSingleton.get(lv, d).showSpellPref();
	}


	void gui_ShowTabular(InsetTabular * it, LyXView & lv, Dialogs & d)
	{
		formTabularSingleton.get(lv, d).showInset(it);
	}


	void gui_UpdateTabular(InsetTabular * it, LyXView & lv, Dialogs & d)
	{
		formTabularSingleton.get(lv, d).updateInset(it);
	}


	void gui_ShowTabularCreate(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlTabularCreate, FormTabularCreate,
			OkApplyCancelReadOnlyPolicy, xformsBC> ctc(lv, d);
		ctc.show();
	}


	void gui_ShowTexinfo(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlTexinfo, FormTexinfo,
			OkCancelPolicy, xformsBC> ct(lv, d);
		ct.show();
	}


	void gui_ShowThesaurus(string const & s, LyXView & lv, Dialogs & d)
	{
#ifdef HAVE_LIBAIKSAURUS
		static GUI<ControlThesaurus, FormThesaurus,
			OkApplyCancelReadOnlyPolicy, xformsBC> ct(lv, d);
		ct.showEntry(s);
#endif
	}


	void gui_ShowTOC(InsetCommand * ic, LyXView & lv, Dialogs & d)
	{
		controlTocSingleton.get(lv, d).showInset(ic);
	}


	void gui_CreateTOC(string const & s, LyXView & lv, Dialogs & d)
	{
		controlTocSingleton.get(lv, d).createInset(s);
	}


	void gui_ShowUrl(InsetCommand * ic, LyXView & lv, Dialogs & d)
	{
		controlUrlSingleton.get(lv, d).showInset(ic);
	}


	void gui_CreateUrl(string const & s, LyXView & lv, Dialogs & d)
	{
		controlUrlSingleton.get(lv, d).createInset(s);
	}


	void gui_ShowVCLogFile(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlVCLog, FormVCLog,
			OkCancelPolicy, xformsBC> cv(lv, d);
		cv.show();
	}

} // extern "C"
