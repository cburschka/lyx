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
#include <boost/smart_ptr.hpp>

#include "math_parinset.h"

class MathMacroTemplate;


/** This class contains the data for a macro
    \author Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
    \version November 1996
 */
class MathMacro : public MathParInset {
public:
	/// A macro can only be builded from an existing template
	explicit
	MathMacro(boost::shared_ptr<MathMacroTemplate> const &);
	/// or from another macro.
	explicit
	MathMacro(MathMacro const &);
	///
	void draw(Painter &, int, int);
	///
	void Metrics();
	///
	MathedInset * Clone();
	///
	void Write(std::ostream &, bool fragile);
	///
	bool setArgumentIdx(int);
	///
	int getArgumentIdx() const;
	///
	int getMaxArgumentIdx() const;
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
	MathedRowSt * getRowSt() const;
	///
	void setData(MathedArray const &);
	///
	MathedTextCodes getTCode() const;
	///
	bool Permit(short) const;
private:
	///
	boost::shared_ptr<MathMacroTemplate> tmplate_;
	///
	struct MacroArgumentBase {
		///
		MathedRowSt * row;
		///
		MathedArray array;
		///
		MacroArgumentBase()
			:  row(0)
			{}
	};
	std::vector<MacroArgumentBase> args_;
	///
	int idx_;
	///
	int nargs_;
	///
	MathedTextCodes tcode_;
};
#endif
