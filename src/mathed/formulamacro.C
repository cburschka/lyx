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
#include "math_matrixinset.h"
#include "lyx_main.h"
#include "BufferView.h"
#include "gettext.h"
#include "Painter.h"
#include "font.h"
#include "support/lyxlib.h"
#include "mathed/support.h"
#include "support/LOstream.h"
#include "debug.h"
#include "lyxlex.h"

using namespace std;

extern MathCursor * mathcursor;

InsetFormulaMacro::InsetFormulaMacro()
	: InsetFormulaBase(new MathMacroTemplate("unknown", 0))
{}


InsetFormulaMacro::InsetFormulaMacro(string nm, int na)
	: InsetFormulaBase(new MathMacroTemplate(nm, na))
{
	MathMacroTable::insertTemplate(tmacro());
}


Inset * InsetFormulaMacro::Clone(Buffer const &) const
{
	return new InsetFormulaMacro(*this);
}


void InsetFormulaMacro::Write(Buffer const *, ostream & os) const
{
	os << "FormulaMacro ";
	tmacro()->Write(os, false);
}


int InsetFormulaMacro::Latex(Buffer const *, ostream & os, bool fragile, 
			     bool /*free_spacing*/) const
{
	tmacro()->Write(os, fragile);
	return 2;
}

int InsetFormulaMacro::Ascii(Buffer const *, ostream & os, int) const
{
	tmacro()->Write(os, false);
	return 0;
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
	// Awful hack...
	par_ = mathed_parse(lex);
	MathMacroTable::insertTemplate(tmacro());
	par_->Metrics(LM_ST_TEXT);
}


string InsetFormulaMacro::prefix() const
{
	return string(" ") + _("Macro: ") + tmacro()->name() + ": ";
}


int InsetFormulaMacro::ascent(BufferView *, LyXFont const &) const
{
	return tmacro()->ascent() + 5;
}


int InsetFormulaMacro::descent(BufferView *, LyXFont const &) const
{
	return tmacro()->descent() + 5;
}


int InsetFormulaMacro::width(BufferView *, LyXFont const & f) const
{
	tmacro()->Metrics(LM_ST_TEXT);
	return 10 + lyxfont::width(prefix(), f) + tmacro()->width();
}


void InsetFormulaMacro::draw(BufferView * bv, LyXFont const & f,
			     int baseline, float & x, bool /*cleared*/) const
{
	Painter & pain = bv->painter();
	LyXFont font(f);

	// label
	font.setColor(LColor::math);
	
	int const y = baseline - ascent(bv, font) + 1;
	int const w = width(bv, font) - 2;
	int const h = ascent(bv, font) + descent(bv, font) - 2;

	pain.fillRectangle(int(x), y , w, h, LColor::mathbg);
	pain.rectangle(int(x), y, w, h, LColor::mathframe);
	
	if (mathcursor && mathcursor->formula() == this && mathcursor->Selection()) {
		int xp[10];
		int yp[10];
		int n;
		mathcursor->SelGetArea(xp, yp, n);
		pain.fillPolygon(xp, yp, n, LColor::selection);
	}

	pain.text(int(x + 2), baseline, prefix(), font);
	x += width(bv, font);

	// formula
	float t = tmacro()->width() + 5;
	x -= t;
	tmacro()->draw(pain, int(x), baseline);
	x += t;
}


UpdatableInset::RESULT
InsetFormulaMacro::LocalDispatch(BufferView * bv,
				 kb_action action, string const & arg)
{
	RESULT result = DISPATCHED;
	switch (action) {
		case LFUN_MATH_MACROARG: {
			int const i = lyx::atoi(arg);
			lyxerr << "inserting macro arg " << i << "\n";
			if (i > 0 && i <= tmacro()->nargs()) {
				mathcursor->insert(new MathMacroArgument(i));
				UpdateLocal(bv);
			} else {
				lyxerr << "not in range 0.." << tmacro()->nargs() << "\n";
			}
			break;
		}
		
		default:
			result = InsetFormulaBase::LocalDispatch(bv, action, arg);
	}
	return result;
}


MathMacroTemplate * InsetFormulaMacro::tmacro() const
{
	return static_cast<MathMacroTemplate *>(par_);
}


Inset::Code InsetFormulaMacro::LyxCode() const
{
	return Inset::MATHMACRO_CODE;
}

