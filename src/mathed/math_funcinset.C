#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_funcinset.h"
#include "lyxfont.h"
#include "font.h"
#include "Painter.h"
#include "mathed/support.h"
#include "support/LOstream.h"


using std::ostream;

extern LyXFont WhichFont(short type, int size);


MathFuncInset::MathFuncInset(string const & nm, MathInsetTypes ot)
	: MathInset(nm, ot)
{
	lims_ = (GetType() == LM_OT_FUNCLIM);
}


MathInset * MathFuncInset::clone() const
{
	return new MathFuncInset(*this);
}


void MathFuncInset::draw(Painter & pain, int x, int y)
{ 
	if (!name_.empty() && name_[0] > ' ') {
		LyXFont font = WhichFont(LM_TC_TEXTRM, size());
#ifndef NO_LATEX
		font.setLatex(LyXFont::ON);
#endif
	        x += (lyxfont::width('I', font) + 3) / 4;
		pain.text(x, y, name_, font);
	}
}


void MathFuncInset::Write(std::ostream & os, bool /* fragile */) const
{
	os << "\\" << name_ << ' ';
}


void MathFuncInset::WriteNormal(std::ostream & os) const
{
	os << "[" << name_ << "] ";
}


void MathFuncInset::Metrics(MathStyles st) 
{
	LyXFont font = WhichFont(LM_TC_TEXTRM, size());
#ifndef NO_LATEX
	font.setLatex(LyXFont::ON);
#endif
	size_ = st;
	if (name_.empty()) {
		width_   = lyxfont::width('M', font);
		ascent_  = lyxfont::ascent('M', font);
		descent_ = 0;
	} else {
		width_ = lyxfont::width(name_, font) + lyxfont::width('I', font) / 2;
		mathed_string_height(LM_TC_TEXTRM, size_, name_, ascent_, descent_);
	}
}
