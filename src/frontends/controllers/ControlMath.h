// -*- C++ -*-
/**
 * \file ControlMath.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROL_MATH_H
#define CONTROL_MATH_H

#include "commandtags.h"
#include "ControlDialog_impl.h"

#include "ButtonController.h"
#include "ButtonPolicies.h"

#include "LString.h"
#include <boost/shared_ptr.hpp>
#include <map>


class GUIMathSub;


class ControlMath : public ControlDialogBD {
public:
	///
	ControlMath(LyXView &, Dialogs &);

	/// dispatch an LFUN
	void dispatchFunc(kb_action act, string const & arg = string()) const;
	/// dispatch a symbol insert
	void insertSymbol(string const & sym, bool bs = true) const;

	///
	void addDaughter(void * key, ViewBase * v, ButtonControllerBase * bc);
	///
	void showDaughter(void *);
	
private:
	///
	virtual void apply();

	///
	typedef boost::shared_ptr<GUIMathSub> DaughterPtr;
	///
	typedef std::map<void *, DaughterPtr> Store;

	/** The store of all daughter dialogs.
	 *  The map uses the button on the main panel to identify them.
	 */
	Store daughters_;

	/// A pointer to the currently active daughter dialog.
	GUIMathSub * active_;
};


class ControlMathSub : public ControlDialogBD {
public:
	///
	ControlMathSub(LyXView &, Dialogs &, ControlMath const & p);

	/// dispatch an LFUN
	void dispatchFunc(kb_action act, string const & arg = string()) const;
	/// dispatch a symbol insert
	void insertSymbol(string const & sym, bool bs = true) const;

private:
	///
	virtual void apply();
	///
	ControlMath const & parent_;
};


class GUIMathSub {
public:
	///
	GUIMathSub(LyXView & lv, Dialogs & d,
		   ControlMath const & p,
		   ViewBase * v,
		   ButtonControllerBase * bc);
	///
	ControlMathSub & controller() { return controller_; }

private:
	///
	ControlMathSub controller_;
	///
	boost::scoped_ptr<ButtonControllerBase> bc_;
	///
	boost::scoped_ptr<ViewBase> view_;
};


extern char const * function_names[];
extern int const nr_function_names;
extern char const * latex_arrow[];
extern int const nr_latex_arrow;
extern char const * latex_bop[];
extern int const nr_latex_bop;
extern char const * latex_brel[];
extern int const nr_latex_brel;
extern char const * latex_dots[];
extern int const nr_latex_dots;
extern char const * latex_greek[];
extern int const nr_latex_greek;
extern char const * latex_deco[];
extern int const nr_latex_deco;
extern char const * latex_misc[];
extern int const nr_latex_misc;
extern char const * latex_varsz[];
extern int const nr_latex_varsz;
extern char const * latex_ams_misc[];
extern int const nr_latex_ams_misc;
extern char const * latex_ams_arrows[];
extern int const nr_latex_ams_arrows;
extern char const * latex_ams_rel[];
extern int const nr_latex_ams_rel;
extern char const * latex_ams_nrel[];
extern int const nr_latex_ams_nrel;
extern char const * latex_ams_ops[];
extern int const nr_latex_ams_ops;

/**
 * Return the mangled XPM filename of the given
 * math symbol.
 */
string const find_xpm(string const & name);
 
#endif /* CONTROL_MATH_H */
