// -*- C++ -*-
/*
 *  File:        math_macro.h
 *  Purpose:     Declaration of macro class for mathed 
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     November 1996
 *  Description: WYSIWYG math macros
 *
 *  Dependencies: Mathed
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *   Version: 0.2, Mathed & Lyx project.
 *
 *   This code is under the GNU General Public Licence version 2 or later.
 */
#ifndef MATH_MACRO
#define MATH_MACRO

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include <iosfwd>
#include <boost/smart_ptr.hpp>

#include "math_parinset.h"
#include "math_macroarg.h"

class MathMacroTemplate;


/** This class contains the data for a macro
    \author Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
    \version November 1996
 */
class MathMacro : public MathParInset {
public:
	/// A macro can be built from an existing template
	explicit MathMacro(MathMacroTemplate const &);
	///
	void draw(Painter &, int, int);
	///
	void Metrics();
	///
	MathedInset * Clone();
	///
	void Write(std::ostream &, bool fragile);
	/// Index 0 is the template, index 1..nargs() are the parameters
	bool setArgumentIdx(int);
	///
	int getArgumentIdx() const;
	///
	int getMaxArgumentIdx() const;
	///
	int nargs() const;
	///
	int GetColumns() const;
	///
	void GetXY(int &, int &) const;
	///
	void SetFocus(int, int);
	///
	MathedArray & GetData();
	///
	MathedArray const & GetData() const;
	///
	void setData(MathedArray const &);
	///
	void setData(MathedArray const &, int);
	///
	MathedTextCodes getTCode() const;
	///
	bool Permit(short) const;
	///
	void expand();
	///
	void dump(std::ostream & os) const;
	///
	MathParInset const * arg(int) const;
	///
	MathParInset * arg(int);
	///
	MathMacroTemplate * tmplate() const;
private:
	///
	MathMacroTemplate * tmplate_;
	/// our arguments
	std::vector< boost::shared_ptr<MathParInset> > args_;
	/// the expanded version fror drawing
	boost::shared_ptr<MathParInset> expanded_;
	///
	int idx_;

	/// unimplemented
	void operator=(MathMacro const &);
};

inline std::ostream & operator<<(std::ostream & os, MathMacro const & m)
{
	m.dump(os);
	return os;
}
#endif
