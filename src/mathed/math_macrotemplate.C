#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macrotemplate.h"
#include "math_macro.h"
#include "macro_support.h"
#include "support/LOstream.h"
#include "support/LAssert.h"
#include "debug.h"
#include "Painter.h"

//using namespace std;

MathMacroTemplate::MathMacroTemplate() :
	MathParInset(LM_ST_TEXT, "undefined", LM_OT_MACRO),
	na_(0), users_()
{}


MathMacroTemplate::MathMacroTemplate(string const & nm, int na) :
	MathParInset(LM_ST_TEXT, nm, LM_OT_MACRO),
	na_(na), users_()
{}


int MathMacroTemplate::nargs() const
{
	return na_;
}


void MathMacroTemplate::WriteDef(ostream & os, bool fragile) const
{
	os << "\n\\newcommand{\\" << name << "}";

	if (na_ > 0)
		os << "[" << na_ << "]";

	os << "{";
#ifdef WITH_WARNINGS
#warning stupid cast
#endif
	const_cast<MathMacroTemplate *>(this)->Write(os, fragile);
	os << "}\n";
}


void MathMacroTemplate::Metrics()
{
	MathParInset::Metrics();
	width   += 4;
	ascent  += 2;
	descent += 2;
}


void MathMacroTemplate::draw(Painter & pain, int x, int y)
{
	MathParInset::draw(pain, x + 2, y + 1);
	int w = Width();
	int a = Ascent();
	int h = Height();
	pain.rectangle(x, y - a, w, h, LColor::blue);
}

