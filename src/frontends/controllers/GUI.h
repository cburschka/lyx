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
				NoRepeatedApplyReadOnlyPolicy, GUIbc>
{
public:
	///
	GUICharacter(LyXView & lv, Dialogs & d)
		: GUI<ControlCharacter, GUIview,
		      NoRepeatedApplyReadOnlyPolicy, GUIbc>(lv, d) {}
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


/** Specialization for Credits dialog
 */
class ControlCredits;

template <class GUIview, class GUIbc>
class GUICredits :
	public GUI<ControlCredits, GUIview, OkCancelPolicy, GUIbc> {
public:
	///
	GUICredits(LyXView & lv, Dialogs & d)
		: GUI<ControlCredits, GUIview, OkCancelPolicy, GUIbc>(lv, d) {}
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
	public GUI<ControlRef, GUIview, NoRepeatedApplyPolicy, GUIbc> {
public:
	///
	GUIRef(LyXView & lv, Dialogs & d)
		: GUI<ControlRef, GUIview, NoRepeatedApplyPolicy, GUIbc>(lv, d) {}
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
