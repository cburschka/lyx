/**
 * \file guiapi.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "guiapi.h"

#include "QAbout.h"
#include "QAboutDialog.h"
#include "QBibitem.h"
#include "QBibitemDialog.h"
#include "QBibtex.h"
#include "QBibtexDialog.h"
#include "QCharacter.h"
#include "QCharacterDialog.h"
#include "QCitation.h"
#include "QCitationDialog.h"
//#include "QDocument.h"
#include "QError.h"
#include "QErrorDialog.h"
#include "QERT.h"
#include "QERTDialog.h"
#include "QExternal.h"
#include "QExternalDialog.h"
#include "QFloat.h"
#include "QFloatDialog.h"
//#include "QForks.h"
// Here would be an appropriate point to lecture on the evils
// of the Qt headers, those most fucked up of disgusting ratholes.
// But I won't.
#undef signals
#include "QGraphics.h"
#include "QGraphicsDialog.h"
#include "QInclude.h"
#include "QIncludeDialog.h"
#include "QIndex.h"
#include "QIndexDialog.h"
#include "QLog.h"
#include "QLogDialog.h"
#include "QMinipage.h"
#include "QMinipageDialog.h"
//#include "QParagraph.h"
#include "QPreamble.h"
#include "QPreambleDialog.h"
//#include "QPreferences.h"
#include "QPrint.h"
#include "QPrintDialog.h"
#include "QRef.h"
#include "QRefDialog.h"
#include "QSearch.h"
#include "QSearchDialog.h"
//#include "QSendto.h"
#include "QShowFile.h"
#include "QShowFileDialog.h"
#include "QSpellchecker.h"
#include "QSpellcheckerDialog.h"
//#include "QTabular.h"
#include "QTabularCreate.h"
#include "QTabularCreateDialog.h"
#include "QTexinfo.h"
#include "QTexinfoDialog.h"
#ifdef HAVE_LIBAIKSAURUS
#include "QThesaurus.h"
#include "QThesaurusDialog.h"
#endif
#include "QToc.h"
#include "QTocDialog.h"
#include "QURL.h"
#include "QURLDialog.h"
#include "QVCLog.h"
#include "QVCLogDialog.h"

#include "Qt2BC.h"

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


GUISingleton<GUI<ControlCharacter, QCharacter,
		 OkApplyCancelReadOnlyPolicy, Qt2BC> >
controlCharacterSingleton;

GUISingleton<GUI<ControlCitation, QCitation,
		 NoRepeatedApplyReadOnlyPolicy, Qt2BC> >
controlCitationSingleton;


GUISingleton<GUI<ControlERT, QERT,
		 NoRepeatedApplyReadOnlyPolicy, Qt2BC> >
controlERTSingleton;

GUISingleton<GUI<ControlIndex, QIndex,
		 NoRepeatedApplyReadOnlyPolicy, Qt2BC> >
controlIndexSingleton;

GUISingleton<GUI<ControlMinipage, QMinipage,
		 NoRepeatedApplyReadOnlyPolicy, Qt2BC> >
controlMinipageSingleton;

//GUISingleton<QPreferences> formPreferencesSingleton;

GUISingleton<GUI<ControlRef, QRef,
		 NoRepeatedApplyReadOnlyPolicy, Qt2BC> >
controlRefSingleton;

//GUISingleton<QTabular> formTabularSingleton;

GUISingleton<GUI<ControlToc, QToc,
		 OkCancelPolicy, Qt2BC> >
controlTocSingleton;

GUISingleton<GUI<ControlUrl, QURL,
		 NoRepeatedApplyReadOnlyPolicy, Qt2BC> >
controlUrlSingleton;

} // anon


extern "C" {

	void gui_ShowAboutlyx(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlAboutlyx, QAbout,
			OkCancelPolicy, Qt2BC> cal(lv, d);
		cal.show();
	}


	void gui_ShowBibitem(InsetCommand * ic, LyXView & lv, Dialogs & d)
	{
		static GUI<ControlBibitem, QBibitem,
			OkCancelReadOnlyPolicy, Qt2BC> cbi(lv, d);
		cbi.showInset(ic);
	}


	void gui_ShowBibtex(InsetCommand * ic, LyXView & lv, Dialogs & d)
	{
		static GUI<ControlBibtex, QBibtex,
			OkCancelReadOnlyPolicy, Qt2BC> cbt(lv, d);
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
		//static QDocument fd(&lv, &d);
		//fd.show();
	}


	void gui_ShowError(InsetError * ie, LyXView & lv, Dialogs & d)
	{
		static GUI<ControlError, QError,
			OkCancelPolicy, Qt2BC> ce(lv, d);
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
		static GUI<ControlExternal, QExternal,
			OkApplyCancelReadOnlyPolicy, Qt2BC> ce(lv, d);
		ce.showInset(ie);
	}


	void gui_ShowFile(string const & f, LyXView & lv, Dialogs & d)
	{
		static GUI<ControlShowFile, QShowFile,
			OkCancelPolicy, Qt2BC> csf(lv, d);
		csf.showFile(f);
	}


	void gui_ShowFloat(InsetFloat * ifl, LyXView & lv, Dialogs & d)
	{
		static GUI<ControlFloat, QFloat,
			NoRepeatedApplyReadOnlyPolicy, Qt2BC> cf(lv, d);
		cf.showInset(ifl);
	}


	void gui_ShowForks(LyXView & lv, Dialogs & d)
	{
#if 0
		static GUI<ControlForks, QForks,
			OkApplyCancelPolicy, Qt2BC> cf(lv, d);
		cf.show();
#endif 
	}


	void gui_ShowGraphics(InsetGraphics * ig, LyXView & lv, Dialogs & d)
	{
		static GUI<ControlGraphics, QGraphics,
			NoRepeatedApplyReadOnlyPolicy, Qt2BC> cg(lv, d);
		cg.showInset(ig);
	}


	void gui_ShowInclude(InsetInclude * ii, LyXView & lv, Dialogs & d)
	{
		static GUI<ControlInclude, QInclude,
			OkCancelReadOnlyPolicy, Qt2BC> ci(lv, d);
		ci.showInset(ii);
	}


	void gui_ShowIndex(InsetCommand * ic, LyXView & lv, Dialogs & d)
	{
		controlIndexSingleton.get(lv, d).showInset(ic);
	}


	void gui_CreateIndex(string const & s, LyXView & lv, Dialogs & d)
	{
		controlIndexSingleton.get(lv, d).createInset(s);
	}


#if 0
	void gui_ShowInfo(InsetInfo *, LyXView & lv, Dialogs & d)
	{
	}
#endif


	void gui_ShowLogFile(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlLog, QLog,
			OkCancelPolicy, Qt2BC> cl(lv, d);
		cl.show();
	}


	void gui_ShowMathPanel(LyXView & lv, Dialogs & d)
	{
		//static QMathsPanel fmp(&lv, &d);
		//fmp.show();
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
#if 0
		static GUI<ControlParagraph, QParagraph,
			OkApplyCancelReadOnlyPolicy, Qt2BC> cp(lv, d);
		cp.show();
#endif
	}


#if 0
	void gui_UpdateParagraph(LyXView & lv, Dialogs & d)
	{
#warning FIXME!
		// This should be the same object as in gui_ShowParagraph
		static GUI<ControlParagraph, QParagraph,
			OkApplyCancelReadOnlyPolicy, Qt2BC> cp(lv, d);
#if 0
		cp.
#endif
	}
#endif


	void gui_ShowPreamble(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlPreamble, QPreamble,
			NoRepeatedApplyReadOnlyPolicy, Qt2BC> cp(lv, d);
		cp.show();
	}


	void gui_ShowPreferences(LyXView & lv, Dialogs & d)
	{
		//formPreferencesSingleton.get(lv, d).show();
	}


	void gui_ShowPrint(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlPrint, QPrint,
			OkApplyCancelPolicy, Qt2BC> cp(lv, d);
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
		static GUI<ControlSearch, QSearch,
			NoRepeatedApplyReadOnlyPolicy, Qt2BC> cs(lv, d);
		cs.show();
	}


	void gui_ShowSendto(LyXView & lv, Dialogs & d)
	{
#if 0
		static GUI<ControlSendto, QSendto,
			OkApplyCancelPolicy, Qt2BC> cs(lv, d);
		cs.show();
#endif 
	}


	void gui_ShowSpellchecker(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlSpellchecker, QSpellchecker,
			NoRepeatedApplyReadOnlyPolicy, Qt2BC> cp(lv, d);
		cp.show();
	}


	void gui_ShowSpellcheckerPreferences(LyXView & lv, Dialogs & d)
	{
		//formPreferencesSingleton.get(lv, d).showSpellPref();
	}


	void gui_ShowTabular(InsetTabular * it, LyXView & lv, Dialogs & d)
	{
		//formTabularSingleton.get(lv, d).showInset(it);
	}


	void gui_UpdateTabular(InsetTabular * it, LyXView & lv, Dialogs & d)
	{
		//formTabularSingleton.get(lv, d).updateInset(it);
	}


	void gui_ShowTabularCreate(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlTabularCreate, QTabularCreate,
			OkApplyCancelReadOnlyPolicy, Qt2BC> ctc(lv, d);
		ctc.show();
	}


	void gui_ShowTexinfo(LyXView & lv, Dialogs & d)
	{
		static GUI<ControlTexinfo, QTexinfo,
			OkCancelPolicy, Qt2BC> ct(lv, d);
		ct.show();
	}


	void gui_ShowThesaurus(string const & s, LyXView & lv, Dialogs & d)
	{
#ifdef HAVE_LIBAIKSAURUS
		static GUI<ControlThesaurus, QThesaurus,
			OkApplyCancelReadOnlyPolicy, Qt2BC> ct(lv, d);
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
		static GUI<ControlVCLog, QVCLog,
			OkCancelPolicy, Qt2BC> cv(lv, d);
		cv.show();
	}

} // extern "C"
