// -*- C++ -*-
/**
 * \file ControlMath.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLMATH_H
#define CONTROLMATH_H


#include "Dialog.h"
#include "lfuns.h" // for kb_action
#include "Font.h"

#include <map>

namespace lyx {
namespace frontend {

struct MathSymbol {
	MathSymbol(char_type uc = '?', unsigned char fc = 0,
		Font::FONT_FAMILY ff = Font::SYMBOL_FAMILY)
		: unicode(uc), fontcode(fc), fontfamily(ff)
	{}
	char_type unicode;
	unsigned char fontcode;
	Font::FONT_FAMILY fontfamily;
};

class ControlMath : public Controller {
public:
	ControlMath(Dialog &);

	/// Nothing to initialise in this case.
	virtual bool initialiseParams(std::string const &) { return true; }
	virtual void clearParams() {}
	virtual void dispatchParams() {}
	virtual bool isBufferDependent() const { return true; }

	/// dispatch an LFUN
	void dispatchFunc(kb_action action, std::string const & arg = std::string()) const;
	/// Insert a math symbol into the doc.
	void dispatchInsert(std::string const & name) const;
	/// Insert a subscript.
	void dispatchSubscript() const;
	/// Insert a superscript.
	void dispatchSuperscript() const;
	/// Insert a cube root
	void dispatchCubeRoot() const;
	/// Insert a matrix
	void dispatchMatrix(std::string const & str) const;
	/// Insert a variable size delimiter
	void dispatchDelim(std::string const & str) const;
	/// Insert a big delimiter
	void dispatchBigDelim(std::string const & str) const;
	/// Switch between display and inline
	void dispatchToggleDisplay() const;
	/** A request to the kernel to launch a dialog.
	 *  \param name the dialog identifier.
	 */
	void showDialog(std::string const & name) const;

	/// \return the math unicode symbol associated to a TeX name.
	MathSymbol const & mathSymbol(std::string tex_name) const;
	/// \return the TeX name associated to a math unicode symbol.
	std::string const & texName(char_type math_symbol) const;

private:
	/// TeX-name / Math-symbol map.
	std::map<std::string, MathSymbol> math_symbols_;
	/// Math-symbol / TeX-name map.
	/// This one is for fast search, it contains the same data as
	/// \c math_symbols_.
	std::map<char_type, std::string> tex_names_;
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
extern char const * latex_delimiters[];
extern int const nr_latex_delimiters;

} // namespace frontend
} // namespace lyx

#endif // NOT CONTROLMATH
