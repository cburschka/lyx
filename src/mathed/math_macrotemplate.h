// -*- C++ -*-
#ifndef MATHMACROTEMPLATE
#define MATHMACROTEMPLATE

#include <vector>

#include <boost/utility.hpp>
//#include <boost/smart_ptr.hpp>

#include "math_parinset.h"
#include "math_macroarg.h"

#ifdef __GNUG__
#pragma interface
#endif

class MathMacro;

/** This class contains the macro definition
    \author Alejandro Aguilar Sierra
 */
class MathMacroTemplate : public MathParInset, public boost::noncopyable {
public:
	friend class MathMacro;
	
	/// A template constructor needs all the data
	explicit
	MathMacroTemplate(string const &, int na);
	///
	void draw(Painter &, int, int);
	///
	void Metrics();
	///
	void WriteDef(std::ostream &, bool fragile);
	/// useful for special insets
	void setTCode(MathedTextCodes t);
	///
	MathedTextCodes getTCode() const;
	/// Number of arguments
	int getNoArgs() const;
	///
	void GetMacroXY(int, int &, int &) const;
	///
	MathParInset * getMacroPar(int) const;
	///
	void setMacroPar(int, MathedArray const &);
	///
	void SetMacroFocus(int &, int, int);
	///
	void setEditMode(bool);
private:
	/// Are we in edit mode or not?
	bool edit_;
	///
	MathedTextCodes tcode_;
	///
	//std::vector<boost::shared_ptr<MathMacroArgument> > args_;
	std::vector<MathMacroArgument> args_;
	///
	int nargs_;
};
#endif
