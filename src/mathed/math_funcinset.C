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


MathFuncInset::MathFuncInset(string const & nm, short ot, short st)
	: MathedInset(nm, ot, st)
{
	lims_ = (GetType() == LM_OT_FUNCLIM);
}


MathedInset * MathFuncInset::Clone()
{
	return new MathFuncInset(name, GetType(), GetStyle());
}


void
MathFuncInset::draw(Painter & pain, int x, int y)
{ 
	if (!name.empty() && name[0] > ' ') {
		LyXFont font = WhichFont(LM_TC_TEXTRM, size());
		font.setLatex(LyXFont::ON);
	        x += (lyxfont::width('I', font) + 3) / 4;
		pain.text(x, y, name, font);
	}
}


void MathFuncInset::Write(std::ostream & os, bool /* fragile */)
{
	os << "\\" << name << ' ';
}


void MathFuncInset::WriteNormal(std::ostream & os)
{
	os << "[" << name << "] ";
}


void MathFuncInset::Metrics() 
{
	LyXFont font = WhichFont(LM_TC_TEXTRM, size());
	font.setLatex(LyXFont::ON);
	if (name.empty()) {
		width = df_width;
		descent = df_des;
		ascent = df_asc;
	} else {
		width = lyxfont::width(name, font)
			+ lyxfont::width('I', font) / 2;
		mathed_string_height(LM_TC_TEXTRM, size(),
				     name, ascent, descent);
	}
}


bool MathFuncInset::GetLimits() const 
{  
	return bool(lims_ && (GetStyle() == LM_ST_DISPLAY)); 
} 
