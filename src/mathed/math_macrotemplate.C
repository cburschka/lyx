#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macrotemplate.h"
#include "Painter.h"
#include "debug.h"


MathMacroTemplate::MathMacroTemplate() :
	MathInset(1, "undefined", LM_OT_MACRO), numargs_(0)
{}


MathMacroTemplate::MathMacroTemplate(string const & nm, int numargs) :
	MathInset(1, nm, LM_OT_MACRO), numargs_(numargs)
{}


MathInset * MathMacroTemplate::clone() const
{
	lyxerr << "cloning MacroTemplate!\n";
	return new MathMacroTemplate(*this);
}


int MathMacroTemplate::numargs() const
{
	return numargs_;
}


void MathMacroTemplate::numargs(int numargs)
{
	numargs_ = numargs;
}


void MathMacroTemplate::Write(std::ostream & os, bool fragile) const
{
	os << "\n\\newcommand{\\" << name_ << "}";

	if (numargs_ > 0)
		os << "[" << numargs_ << "]";

	os << "{";
	cell(0).Write(os, fragile);
	os << "}\n";
}


void MathMacroTemplate::Metrics(MathStyles st)
{
	xcell(0).Metrics(st);
	size_    = st;
	width_   = xcell(0).width() + 4;
	ascent_  = xcell(0).ascent() + 2;
	descent_ = xcell(0).descent() + 2;
}


void MathMacroTemplate::draw(Painter & pain, int x, int y)
{
	xo(x);
	yo(y);
	xcell(0).draw(pain, x + 2, y + 1);
	pain.rectangle(x, y - ascent(), width(), height(), LColor::blue);
}
