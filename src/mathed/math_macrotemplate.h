// -*- C++ -*-
#ifndef MATHMACROTEMPLATE
#define MATHMACROTEMPLATE

#include <set>

#include "math_parinset.h"

#ifdef __GNUG__
#pragma interface
#endif

class MathMacro;

/** This class contains the macro definition
    \author Alejandro Aguilar Sierra
 */
//class MathMacroTemplate : public MathParInset, boost::noncopyable 

class MathMacroTemplate : public MathParInset {
public:
	///
	MathMacroTemplate();
	///
	MathMacroTemplate(std::string const & name, int nargs);
	///
	void WriteDef(std::ostream &, bool fragile) const;
	/// Number of arguments
	int nargs() const;
	///
	void draw(Painter &, int, int);
	///
	void Metrics();
private:
	///
	int na_;
	///
	std::set<MathMacro *> users_;

	/// unimplemented
	void operator=(MathMacroTemplate const &);
	/// unimplemented
	MathMacroTemplate(MathMacroTemplate const &);
};

#endif
