/*
 * \file ControlCopyright.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifndef CONTROLCOPYRIGHT_H
#define CONTROLCOPYRIGHT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialogs.h"

/** A controller for Copyright dialogs.
 */
class ControlCopyright : public ControlDialog<ControlConnectBI> {
public:
	///
	ControlCopyright(LyXView &, Dialogs &);

	///
	string const getCopyright() const;
	///
	string const getLicence() const;
	///
	string const getDisclaimer() const;

private:
	/// not needed.
	virtual void apply() {}
};

/** This class instantiates and makes available the GUI-specific
    ButtonController and View.
 */
template <class GUIview, class GUIbc>
class GUICopyright : public ControlCopyright {
public:
	///
	GUICopyright(LyXView &, Dialogs &);
	///
	virtual ButtonControllerBase & bc() { return bc_; }
	///
	virtual ViewBase & view() { return view_; }

private:
	///
	ButtonController<OkCancelPolicy, GUIbc> bc_;
	///
	GUIview view_;
};

template <class GUIview, class GUIbc>
GUICopyright<GUIview, GUIbc>::GUICopyright(LyXView & lv, Dialogs & d)
	: ControlCopyright(lv, d),
	  view_(*this)
{}
#endif // CONTROLCOPYRIGHT_H

