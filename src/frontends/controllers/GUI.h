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
#include "ButtonPolicies.h"
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


/** This class instantiates and makes available the GUI-specific
    ButtonController and View.
 */
template <class Controller, class GUIview, class Policy, class GUIbc>
class GUI : public Controller {
public:
	///
	GUI(LyXView & lv, Dialogs & d) : Controller(lv, d), view_(*this) {}
	///
	virtual ButtonControllerBase & bc() { return bc_; }
	///
	virtual ViewBase & view() { return view_; }

private:
	///
	ButtonController<Policy, GUIbc> bc_;
	///
	GUIview view_;
};

/** Specialization for About LyX dialog
 */
template <class GUIview, class GUIbc>
class GUIAboutlyx :
	public GUI<ControlAboutlyx, GUIview, OkCancelPolicy, GUIbc> {
public:
	///
	GUIAboutlyx(LyXView & lv, Dialogs & d)
		: GUI<ControlAboutlyx, GUIview, OkCancelPolicy, GUIbc>(lv, d) {}
};

/** Specialization for Bibitem dialog
 */
template <class GUIview, class GUIbc>
class GUIBibitem :
	public GUI<ControlBibitem, GUIview, OkCancelReadOnlyPolicy, GUIbc> {
public:
	///
	GUIBibitem(LyXView & lv, Dialogs & d)
		: GUI<ControlBibitem, GUIview, OkCancelReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Bibtex dialog
 */
template <class GUIview, class GUIbc>
class GUIBibtex :
	public GUI<ControlBibtex, GUIview, OkCancelReadOnlyPolicy, GUIbc> {
public:
	///
	GUIBibtex(LyXView & lv, Dialogs & d)
		: GUI<ControlBibtex, GUIview, OkCancelReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Character dialog
 */
template <class GUIview, class GUIbc>
class GUICharacter : public GUI<ControlCharacter, GUIview,
				OkApplyCancelReadOnlyPolicy, GUIbc>
{
public:
	///
	GUICharacter(LyXView & lv, Dialogs & d)
		: GUI<ControlCharacter, GUIview,
		      OkApplyCancelReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Citation dialog
 */
template <class GUIview, class GUIbc>
class GUICitation : public GUI<ControlCitation, GUIview,
			       NoRepeatedApplyReadOnlyPolicy, GUIbc>
{
public:
	///
	GUICitation(LyXView & lv, Dialogs & d)
		: GUI<ControlCitation, GUIview,
		      NoRepeatedApplyReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Error dialog
 */
template <class GUIview, class GUIbc>
class GUIError :
	public GUI<ControlError, GUIview, OkCancelPolicy, GUIbc> {
public:
	///
	GUIError(LyXView & lv, Dialogs & d)
		: GUI<ControlError, GUIview, OkCancelPolicy, GUIbc>(lv, d) {}
};


/** Specialization for ERT dialog
 */
template <class GUIview, class GUIbc>
class GUIERT :
	public GUI<ControlERT, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc> {
public:
	///
	GUIERT(LyXView & lv, Dialogs & d)
		: GUI<ControlERT, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for External dialog
 */
template <class GUIview, class GUIbc>
class GUIExternal :
	public GUI<ControlExternal, GUIview, OkApplyCancelReadOnlyPolicy, GUIbc> {
public:
	///
	GUIExternal(LyXView & lv, Dialogs & d)
		: GUI<ControlExternal, GUIview, OkApplyCancelReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Forks dialog
 */
template <class GUIview, class GUIbc>
class GUIForks :
	public GUI<ControlForks, GUIview, OkApplyCancelPolicy, GUIbc> {
public:
	///
	GUIForks(LyXView & lv, Dialogs & d)
		: GUI<ControlForks, GUIview, OkApplyCancelPolicy, GUIbc>(lv, d) {}
};

/** Specialization for Graphics dialog
 */
template <class GUIview, class GUIbc>
class GUIGraphics :
	public GUI<ControlGraphics, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc> {
public:
	///
	GUIGraphics(LyXView & lv, Dialogs & d)
		: GUI<ControlGraphics, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Include dialog
 */
template <class GUIview, class GUIbc>
class GUIInclude :
	public GUI<ControlInclude, GUIview, OkCancelReadOnlyPolicy, GUIbc> {
public:
	///
	GUIInclude(LyXView & lv, Dialogs & d)
		: GUI<ControlInclude, GUIview, OkCancelReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Index dialog
 */
template <class GUIview, class GUIbc>
class GUIIndex :
	public GUI<ControlIndex, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc> {
public:
	///
	GUIIndex(LyXView & lv, Dialogs & d)
		: GUI<ControlIndex, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Log dialog
 */
template <class GUIview, class GUIbc>
class GUILog :
	public GUI<ControlLog, GUIview, OkCancelPolicy, GUIbc> {
public:
	///
	GUILog(LyXView & lv, Dialogs & d)
		: GUI<ControlLog, GUIview, OkCancelPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Minipage dialog
 */
template <class GUIview, class GUIbc>
class GUIMinipage :
	public GUI<ControlMinipage, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc> {
public:
	///
	GUIMinipage(LyXView & lv, Dialogs & d)
		: GUI<ControlMinipage, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Float dialog
 */
template <class GUIview, class GUIbc>
class GUIFloat :
	public GUI<ControlFloat, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc> {
public:
	///
	GUIFloat(LyXView & lv, Dialogs & d)
		: GUI<ControlFloat, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Preamble dialog
 */
template <class GUIview, class GUIbc>
class GUIPreamble :
	public GUI<ControlPreamble, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc> {
public:
	///
	GUIPreamble(LyXView & lv, Dialogs & d)
		: GUI<ControlPreamble, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Print dialog
 */
template <class GUIview, class GUIbc>
class GUIPrint :
	public GUI<ControlPrint, GUIview, OkApplyCancelPolicy, GUIbc> {
public:
	///
	GUIPrint(LyXView & lv, Dialogs & d)
		: GUI<ControlPrint, GUIview, OkApplyCancelPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Ref dialog
 */
template <class GUIview, class GUIbc>
class GUIRef :
	public GUI<ControlRef, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc> {
public:
	///
	GUIRef(LyXView & lv, Dialogs & d)
		: GUI<ControlRef, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Search dialog
 */
template <class GUIview, class GUIbc>
class GUISearch :
	public GUI<ControlSearch, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc> {
public:
	///
	GUISearch(LyXView & lv, Dialogs & d)
		: GUI<ControlSearch, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Sendto dialog
 */
template <class GUIview, class GUIbc>
class GUISendto :
	public GUI<ControlSendto, GUIview, OkApplyCancelPolicy, GUIbc> {
public:
	///
	GUISendto(LyXView & lv, Dialogs & d)
		: GUI<ControlSendto, GUIview, OkApplyCancelPolicy, GUIbc>(lv, d) {}
};


/** Specialization for ShowFile dialog
 */
template <class GUIview, class GUIbc>
class GUIShowFile :
	public GUI<ControlShowFile, GUIview, OkCancelPolicy, GUIbc> {
public:
	///
	GUIShowFile(LyXView & lv, Dialogs & d)
		: GUI<ControlShowFile, GUIview, OkCancelPolicy, GUIbc>(lv, d) {}
};

/** Specialization for Spellchecker dialog
 */
template <class GUIview, class GUIbc>
class GUISpellchecker :
	public GUI<ControlSpellchecker, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc> {
public:
	///
	GUISpellchecker(LyXView & lv, Dialogs & d)
		: GUI<ControlSpellchecker, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc>(lv, d) {}
};

/** Specialization for Toc dialog
 */
template <class GUIview, class GUIbc>
class GUIToc :
	public GUI<ControlToc, GUIview, OkCancelPolicy, GUIbc> {
public:
	///
	GUIToc(LyXView & lv, Dialogs & d)
		: GUI<ControlToc, GUIview, OkCancelPolicy, GUIbc>(lv, d) {}
};


/** Specialization for TabularCreate dialog
 */
template <class GUIview, class GUIbc>
class GUITabularCreate :
	public GUI<ControlTabularCreate, GUIview,
		   OkApplyCancelReadOnlyPolicy, GUIbc> {
public:
	///
	GUITabularCreate(LyXView & lv, Dialogs & d)
		: GUI<ControlTabularCreate, GUIview,
		      OkApplyCancelReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Texinfo dialog
 */
template <class GUIview, class GUIbc>
class GUITexinfo :
	public GUI<ControlTexinfo, GUIview, OkCancelPolicy, GUIbc> {
public:
	///
	GUITexinfo(LyXView & lv, Dialogs & d)
		: GUI<ControlTexinfo, GUIview, OkCancelPolicy, GUIbc>(lv, d) {}
};

/** Specialization for Thesaurus dialog
 */
template <class GUIview, class GUIbc>
class GUIThesaurus :
	public GUI<ControlThesaurus, GUIview,
		   OkApplyCancelReadOnlyPolicy, GUIbc> {
public:
	///
	GUIThesaurus(LyXView & lv, Dialogs & d)
		: GUI<ControlThesaurus, GUIview,
		      OkApplyCancelReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Url dialog
 */
template <class GUIview, class GUIbc>
class GUIUrl :
	public GUI<ControlUrl, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc> {
public:
	///
	GUIUrl(LyXView & lv, Dialogs & d)
	    : GUI<ControlUrl, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for VCLog dialog
 */
template <class GUIview, class GUIbc>
class GUIVCLog :
	public GUI<ControlVCLog, GUIview, OkCancelPolicy, GUIbc> {
public:
	///
	GUIVCLog(LyXView & lv, Dialogs & d)
	    : GUI<ControlVCLog, GUIview, OkCancelPolicy, GUIbc>(lv, d) {}
};


#endif // GUI_H
