// -*- C++ -*-
#ifndef MATHMACROTEMPLATE
#define MATHMACROTEMPLATE

#include <vector>

#include "math_parinset.h"
#include "math_macroarg.h"

#ifdef __GNUG__
#pragma interface
#endif

class MathMacro;

/** This class contains the macro definition
    \author Alejandro Aguilar Sierra
 */
class MathMacroTemplate : public MathParInset {
public:
	/// A template constructor needs all the data
	explicit
	MathMacroTemplate(string const &, int na = 0, int f = 0);
	///
	//~MathMacroTemplate();
	///
	void draw(Painter &, int, int);
	///
	void Metrics();
	///
	void WriteDef(std::ostream &, bool fragile);
	/// useful for special insets
	void  setTCode(MathedTextCodes t);
	///
	MathedTextCodes getTCode() const;
	/// 
	void setArgument(MathedArray *, int i= 0);
	/// Number of arguments
	int getNoArgs() const;
	///
	void GetMacroXY(int, int &, int &) const;
	///
	MathParInset * getMacroPar(int) const;
	///
	void SetMacroFocus(int &, int, int);
	///
	void setEditMode(bool);
	
	/// Replace the appropriate arguments with a specific macro's data
	void update(MathMacro * m = 0);
	///
	short flags() const;
private:
	///
	short flags_;
	///
	MathedTextCodes tcode_;
	///
	std::vector<MathMacroArgument> args_;
	///
	int nargs_;
};


inline
short MathMacroTemplate::flags() const {
	return flags_;
}
#endif
