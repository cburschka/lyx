/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Lsstream.h"
#include "Spacing.h"
#include "LString.h"

using std::ios;
using std::ostream;

/// how can I put this inside of Spacing (class)
static
char const * spacing_string[] = {"single", "onehalf", "double", "other"};


float Spacing::getValue() const 
{
	switch (space) {
	case Default: // nothing special should happen with this...
	case Single: return 1.0;
	case Onehalf: return 1.25;
	case Double: return 1.667;
	case Other: return value;
	}
	return 1.0;
}


void Spacing::set(Spacing::Space sp, float val)
{
	space = sp;
	if (sp == Other) {
		switch (int(val * 1000 + 0.5)) {
		case 1000: space = Single; break;
		case 1250: space = Onehalf; break;
		case 1667: space = Double; break;
		default: value = val; break;
		}
	}
}


void Spacing::set(Spacing::Space sp, string const & val)
{
	float fval;
	istringstream istr(val.c_str());
	istr >> fval;
	set(sp, fval);
}


void Spacing::writeFile(ostream & os, bool para) const
{
	if (space == Default) return;
	
	string cmd = para ? "\\paragraph_spacing " : "\\spacing ";
	
	if (getSpace() == Spacing::Other) {
		os.setf(ios::showpoint|ios::fixed);
		os.precision(2);
		os << cmd << spacing_string[getSpace()]
		   << " " << getValue() << " \n";
	} else {
		os << cmd << spacing_string[getSpace()] << " \n";
	}	
}


string const Spacing::writeEnvirBegin() const
{
	switch (space) {
	case Default: break; // do nothing
	case Single:
		return "\\begin{singlespace}";
	case Onehalf:
		return "\\begin{onehalfspace}";
	case Double:
		return "\\begin{doublespace}";
	case Other:
	{
		ostringstream ost;
		ost << "\\begin{spacing}{"
		    << getValue() << "}";
		return ost.str().c_str();
	}
	}
	return string();
}


string const Spacing::writeEnvirEnd() const
{
	switch (space) {
	case Default: break; // do nothing
	case Single:
		return "\\end{singlespace}";
	case Onehalf:
		return "\\end{onehalfspace}";
	case Double:
		return "\\end{doublespace}";
	case Other:
		return "\\end{spacing}";
	}
	return string();
}
