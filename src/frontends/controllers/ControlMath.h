// -*- C++ -*-
/**
 * \file ControlMath.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 *
 * ControlMath2 is a controller class for the Math Panel dialog.
 */

#ifndef CONTROLMATH_H
#define CONTROLMATH_H


#include "Dialog.h"
#include "lfuns.h" // for kb_action


class ControlMath : public Dialog::Controller {
public:
	ControlMath(Dialog &);

	/// Nothing to initialise in this case.
	virtual bool initialiseParams(string const &) { return true; }
	virtual void clearParams() {}
	virtual void dispatchParams() {}
	virtual bool isBufferDependent() const { return true; }

	/// dispatch an LFUN
	void dispatchFunc(kb_action action, string const & arg = string()) const;
	/// Insert a math symbol into the doc.
	void dispatchInsert(string const & name) const;
	/// Insert a subscript.
	void dispatchSubscript() const;
	/// Insert a superscript.
	void dispatchSuperscript() const;
	/// Insert a cube root
	void dispatchCubeRoot() const;
	/// Insert a matrix
	void dispatchMatrix(string const & str) const;
	/// Insert a delimiter
	void dispatchDelim(string const & str) const;
	/// Wwitch between display and inline
	void dispatchToggleDisplay() const;
	/** A request to the kernel to launch a dialog.
	 *  \param name the dialog identifier.
	 */
	void showDialog(string const & name) const;
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

#endif // NOT CONTROLMATH
