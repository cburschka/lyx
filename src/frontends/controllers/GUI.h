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

/** This class instantiates and makes available the GUI-specific
    View for the Splash screen controller.
 */
template <class GUIview>
class GUISplash : public ControlSplash {
public:
	///
	GUISplash(Dialogs & d) : ControlSplash(d), view_(*this) {}
	///
	virtual ViewSplash & view() { return view_; }

private:
	///
	GUIview view_;
};


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

/// Forward declaration of ButtonPolicies
class OkCancelPolicy;
class OkCancelReadOnlyPolicy;
class NoRepeatedApplyReadOnlyPolicy;

/** Specialization for About LyX dialog
 */
class ControlAboutlyx;

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
class ControlBibitem;

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
class ControlBibtex;

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
class ControlCharacter;

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
class ControlCitation;

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
class ControlError;

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
class ControlERT;

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
class ControlExternal;

template <class GUIview, class GUIbc>
class GUIExternal :
	public GUI<ControlExternal, GUIview, OkCancelReadOnlyPolicy, GUIbc> {
public:
	///
	GUIExternal(LyXView & lv, Dialogs & d)
		: GUI<ControlExternal, GUIview, OkCancelReadOnlyPolicy, GUIbc>(lv, d) {}
};


/** Specialization for Graphics dialog
 */
class ControlGraphics;

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
class ControlInclude;

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
class ControlIndex;

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
class ControlLog;

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
class ControlMinipage;

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
class ControlFloat;

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
class ControlPreamble;

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
class ControlPrint;

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
class ControlRef;

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
class ControlSearch;

template <class GUIview, class GUIbc>
class GUISearch :
	public GUI<ControlSearch, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc> {
public:
	///
	GUISearch(LyXView & lv, Dialogs & d)
		: GUI<ControlSearch, GUIview, NoRepeatedApplyReadOnlyPolicy, GUIbc>(lv, d) {}
};

/** Specialization for Spellchecker dialog
 */
class ControlSpellchecker;

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
class ControlToc;

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
class ControlTabularCreate;

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


/** Specialization for Thesaurus dialog
 */
class ControlThesaurus;

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
class ControlUrl;

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
class ControlVCLog;

template <class GUIview, class GUIbc>
class GUIVCLog :
	public GUI<ControlVCLog, GUIview, OkCancelPolicy, GUIbc> {
public:
	///
	GUIVCLog(LyXView & lv, Dialogs & d)
	    : GUI<ControlVCLog, GUIview, OkCancelPolicy, GUIbc>(lv, d) {}
};


#endif // GUI_H
