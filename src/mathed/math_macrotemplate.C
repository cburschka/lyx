#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macrotemplate.h"
#include "math_mathmlstream.h"
#include "math_parser.h"
#include "frontends/Painter.h"
#include "debug.h"


MathMacroTemplate::MathMacroTemplate()
	: MathNestInset(2), numargs_(0), name_()
{}


MathMacroTemplate::MathMacroTemplate(string const & nm, int numargs,
		MathArray const & ar1, MathArray const & ar2)
	: MathNestInset(2), numargs_(numargs), name_(nm)
{
	if (numargs_ > 9)
		lyxerr << "MathMacroTemplate::MathMacroTemplate: wrong # of arguments: "
			<< numargs_ << std::endl;
	cell(0) = ar1;
	cell(1) = ar2;
}



MathMacroTemplate::MathMacroTemplate(std::istream & is)
	: MathNestInset(2), numargs_(0), name_()
{
	MathArray ar;
	mathed_parse_cell(ar, is);
	if (ar.size() != 1 || !ar[0]->asMacroTemplate()) {
		lyxerr << "cannot read macro from '" << ar << "'\n";
		return;
	}
	operator=( *(ar[0]->asMacroTemplate()) );
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


string MathMacroTemplate::name() const
{
	return name_;
}


void MathMacroTemplate::metrics(MathMetricsInfo & mi) const
{
	cell(0).metrics(mi);
	cell(1).metrics(mi);
	dim_.w = cell(0).width() + cell(1).width() + 10;
	dim_.a = std::max(cell(0).ascent(),  cell(1).ascent())  + 2;
	dim_.d = std::max(cell(0).descent(), cell(1).descent()) + 2;
}


void MathMacroTemplate::draw(MathPainterInfo & pi, int x, int y) const
{
	int const w0 = cell(0).width();
	int const w1 = cell(1).width();
	cell(0).draw(pi, x + 2, y + 1);
	pi.pain.rectangle(x, y - ascent() + 1, w0 + 4, height(),
			LColor::blue);
	cell(1).draw(pi, x + 8 + w0, y + 1);
	pi.pain.rectangle(x + w0 + 6 , y - ascent() + 1, w1 + 4,
			height(), LColor::blue);
}



void MathMacroTemplate::write(WriteStream & os) const
{
	if (os.latex()) {
		os << "\n\\newcommand{\\" << name_.c_str() << '}';
		if (numargs_ > 0)
			os << '[' << numargs_ << ']';
		os << '{' << cell(0) << "}\n";
	} else {
		// writing .lyx
		os << "\n\\newcommand{\\" << name_.c_str() << '}';
		if (numargs_ > 0)
			os << '[' << numargs_ << ']';
		os << '{' << cell(0) << '}';
		// write special .tex export only if necessary
		if (!cell(1).empty())
			os << "\n{" << cell(1) << '}';
	}
}
