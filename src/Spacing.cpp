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

#include <ostream>

using namespace std;

namespace lyx {


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


namespace {

string envName(Spacing::Space space, bool useSetSpace) 
{
	static char const * const env_names[]
		= { "SingleSpace", "OnehalfSpace", "DoubleSpace", "Spacing", ""};
	string const name = env_names[space];

	return useSetSpace ? name : support::ascii_lowercase(name);
}

}

string const Spacing::writeEnvirBegin(bool useSetSpace) const
{
	string const name = envName(space, useSetSpace);
	if (space == Other) 
		return "\\begin{" + name + "}{" + getValueAsString() + '}';
	else 
		return name.empty() ? string() : "\\begin{" + name + '}';
}


string const Spacing::writeEnvirEnd(bool useSetSpace) const
{
	string const name = envName(space, useSetSpace);
	return name.empty() ? string() : "\\end{" + name + '}';
}


string const Spacing::writePreamble(bool useSetSpace) const
{
	string preamble;
	switch (space) {
	case Default:
	case Single:
		// we dont use setspace.sty so dont print anything
		//return "\\singlespacing\n";
		break;
	case Onehalf:
		preamble = useSetSpace ? "\\OnehalfSpacing\n" 
			: "\\onehalfspacing\n";
		break;
	case Double:
		preamble = useSetSpace ? "\\DoubleSpacing\n" 
			: "\\doublespacing\n";
		break;
	case Other:
		preamble = (useSetSpace ? "\\setSpacing{" : "\\setstretch{")
			+ getValueAsString() + "}\n";
		break;
	}
	return preamble;
}

} // namespace lyx
