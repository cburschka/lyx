#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macrotemplate.h"
#include "Painter.h"
#include "debug.h"


MathMacroTemplate::MathMacroTemplate()
	: MathNestInset(1), numargs_(0), name_()
{}


MathMacroTemplate::MathMacroTemplate(string const & nm, int numargs)
	: MathNestInset(1), numargs_(numargs), name_(nm)
{
	if (numargs_ > 9) {
		lyxerr << "MathMacroTemplate::MathMacroTemplate: wrong # of arguments: "
			<< numargs_ << std::endl;
	}
}


MathInset * MathMacroTemplate::clone() const
{
	//lyxerr << "cloning MacroTemplate!\n";
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


string const & MathMacroTemplate::name() const
{
	return name_;
}


void MathMacroTemplate::write(MathWriteInfo & os) const
{
	os << "\n\\newcommand{\\" << name_ << '}';
	if (numargs_ > 0)
		os << '[' << numargs_ << ']';
	os << '{' << cell(0) << "}\n";
}


void MathMacroTemplate::metrics(MathMetricsInfo const & mi) const
{
	xcell(0).metrics(mi);
	width_   = xcell(0).width() + 4;
	ascent_  = xcell(0).ascent() + 2;
	descent_ = xcell(0).descent() + 2;
}


void MathMacroTemplate::draw(Painter & pain, int x, int y) const
{
	xcell(0).draw(pain, x + 2, y + 1);
	pain.rectangle(x, y - ascent(), width(), height(), LColor::blue);
}
