/*
 *  File:        formula.h
 *  Purpose:     Implementation of formula inset
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Allows the edition of math paragraphs inside Lyx. 
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *  Version: 0.4, Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "formulamacro.h"
#include "commandtags.h"
#include "math_cursor.h"
#include "math_parser.h"
#include "math_macro.h"
#include "math_macroarg.h"
#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "lyx_main.h"
#include "BufferView.h"
#include "gettext.h"
#include "Painter.h"
#include "font.h"
#include "support/lyxlib.h"
#include "mathed/support.h"
#include "support/LOstream.h"
#include "debug.h"

using std::ostream;
using std::istream;

extern MathedCursor * mathcursor;

InsetFormulaMacro::InsetFormulaMacro()
	: InsetFormula(true)
{
	par = &MathMacroTable::provideTemplate("unknown", 0);
}


InsetFormulaMacro::InsetFormulaMacro(string nm, int na)
  : InsetFormula(true)
{
	par = &MathMacroTable::provideTemplate(nm, na);
}


Inset * InsetFormulaMacro::Clone(Buffer const &) const
{
	return new InsetFormulaMacro(*this);
}


void InsetFormulaMacro::Write(Buffer const *, ostream & os) const
{
	os << "FormulaMacro ";
	tmacro()->WriteDef(os, false);
}


int InsetFormulaMacro::Latex(Buffer const *, ostream & os, bool /*fragile*/, 
			     bool /*free_spacing*/) const
{
	tmacro()->WriteDef(os, true); // or false?
	return 2;
}


int InsetFormulaMacro::Linuxdoc(Buffer const * buf, ostream & os) const
{
	return Ascii(buf, os, 0);
}


int InsetFormulaMacro::DocBook(Buffer const * buf, ostream & os) const
{
	return Ascii(buf, os, 0);
}


void InsetFormulaMacro::Read(Buffer const *, LyXLex & lex)
{
	istream & is = lex.getStream();
	mathed_parser_file(is, lex.GetLineNo());

	MathParInset * tmp = new MathParInset;
	MathedArray ar;
	mathed_parse(ar, tmp, 0);
	par = &MathMacroTable::provideTemplate(tmp->GetName(), tmp->xo());
	par->setData(ar);
	//cerr << "## InsetFormulaMacro::Read name: " << tmp->GetName() << endl;
	//cerr << "## InsetFormulaMacro::Read nargs: " << tmp->xo() << endl;
	//cerr << "## InsetFormulaMacro::Read 1: " << ar << endl;
	
	// Update line number
	lex.setLineNo(mathed_parser_lineno());
	
	// reading of end_inset in the inset!!!
	while (lex.IsOK()) {
		lex.nextToken();
		if (lex.GetString() == "\\end_inset")
			break;
	}
}

int InsetFormulaMacro::ascent(BufferView * pain, LyXFont const & f) const
{
	return InsetFormula::ascent(pain, f);
}


int InsetFormulaMacro::descent(BufferView * pain, LyXFont const & f) const
{
	return InsetFormula::descent(pain, f);
}


string InsetFormulaMacro::prefix() const
{
	return string(" ") + _("Macro: ") + par->GetName() + ": ";
}

int InsetFormulaMacro::width(BufferView * bv, LyXFont const & f) const
{
	return 10 + lyxfont::width(prefix(), f) + InsetFormula::width(bv, f);
}


void InsetFormulaMacro::draw(BufferView * bv, LyXFont const & f,
			     int baseline, float & x, bool cleared) const
{
	Painter & pain = bv->painter();
	LyXFont font(f);

	// label
	font.setColor(LColor::math);
	
	int const y = baseline - ascent(bv, font) + 1;
	int const w = width(bv, font) - 2;
	int const h = (ascent(bv, font) + descent(bv, font) - 2);

	pain.fillRectangle(int(x), y, w, h, LColor::mathbg);
	pain.rectangle(int(x), y, w, h, LColor::mathframe);
	
	pain.text(int(x + 2), baseline, prefix(), font);
	x += width(bv, font);

	// formula
	float t = InsetFormula::width(bv, f) + 5;
	x -= t;
	InsetFormula::draw(bv, font, baseline, x, cleared);
	x += t;
}


string const InsetFormulaMacro::EditMessage() const 
{
	return _("Math macro editor mode");
}


UpdatableInset::RESULT
InsetFormulaMacro::LocalDispatch(BufferView * bv,
				 kb_action action, string const & arg)
{
	if (action == LFUN_MATH_MACROARG) {
		int const i = lyx::atoi(arg);
		if (i > 0 && i <= tmacro()->nargs()) {
			mathcursor->insertInset(new MathMacroArgument(i), LM_TC_INSET);
			InsetFormula::UpdateLocal(bv);
		}
	
		return DISPATCHED;
	}
	par->Metrics();
	return InsetFormula::LocalDispatch(bv, action, arg);
}

MathMacroTemplate * InsetFormulaMacro::tmacro() const
{
	return static_cast<MathMacroTemplate *>(par);
}
