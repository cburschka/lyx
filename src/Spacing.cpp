/**
 * \file Spacing.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Spacing.h"
#include "support/lstrings.h"
#include "support/convert.h"

#include <sstream>
#include <string>


namespace lyx {


using std::ostream;
using std::ostringstream;
using std::string;


string const Spacing::spacing_string[]
	= {"single", "onehalf", "double", "other"};


string const Spacing::getValueAsString() const
{
	switch (space) {
	case Default: // nothing special should happen with this...
	case Single: return "1.0";
	case Onehalf: return "1.25";
	case Double: return "1.667";
	case Other: return value;
	}
	return "1.0";
}


double Spacing::getValue() const
{
	return convert<double>(getValueAsString());
}


void Spacing::set(Spacing::Space sp, double val)
{
	set(sp, convert<string>(val));
}


void Spacing::set(Spacing::Space sp, string const & val)
{
	space = sp;
	if (sp == Other) {
		switch (int(convert<double>(val) * 1000 + 0.5)) {
		case 1000:
			space = Single;
			break;
		case 1250:
			space = Onehalf;
			break;
		case 1667:
			space = Double;
			break;
		default:
			value = val;
			break;
		}
	}
}


void Spacing::writeFile(ostream & os, bool para) const
{
	if (space == Default) return;

	string cmd = para ? "\\paragraph_spacing " : "\\spacing ";

	if (getSpace() == Spacing::Other) {
		os << cmd << spacing_string[getSpace()]
		   << ' ' << getValueAsString() << "\n";
	} else {
		os << cmd << spacing_string[getSpace()] << "\n";
	}
}


string const Spacing::writeEnvirBegin(bool useSetSpace) const
{
	switch (space) {
	case Default: break; // do nothing
	case Single:
		return (useSetSpace ? "\\begin{SingleSpace}"
			: "\\begin{singlespace}");
	case Onehalf:
		return (useSetSpace ? "\\begin{OnehalfSpace}"
			: "\\begin{onehalfspace}");
	case Double:
		return (useSetSpace ? "\\begin{DoubleSpace}" 
			: "\\begin{doublespace}");
	case Other:
	{
		ostringstream ost;
		ost << (useSetSpace ? "\\begin{Spacing}{"
			: "\\begin{spacing}{" )
		    << getValueAsString() << '}';
		return ost.str();
	}
	}
	return string();
}


string const Spacing::writeEnvirEnd(bool useSetSpace) const
{
	switch (space) {
	case Default: break; // do nothing
	case Single:
		return (useSetSpace ? "\\end{SingleSpace}"
			: "\\end{singlespace}");
	case Onehalf:
		return (useSetSpace ? "\\end{OnehalfSpace}"
			: "\\end{onehalfspace}");
	case Double:
		return (useSetSpace ? "\\end{DoubleSpace}" 
			: "\\end{doublespace}");
	case Other:
		return (useSetSpace ? "\\end{Spacing}" : "\\end{spacing}") ;
	}
	return string();
}


} // namespace lyx
