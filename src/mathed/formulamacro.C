/*
 *  File:        formulamacro.C
 *  Purpose:     Implementation of the formula macro LyX inset
 *  Author:      André Pönitz
 *  Created:     March 2001
 *  Description: Allows the edition of math macros inside Lyx. 
 *
 *  Copyright: 2001  The LyX Project
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
#include "lyxfont.h"

using std::ostream;

extern MathCursor * mathcursor;

InsetFormulaMacro::InsetFormulaMacro()
	: InsetFormulaBase(new MathMacroTemplate("unknown", 0))
{}


InsetFormulaMacro::InsetFormulaMacro(string nm, int na)
	: InsetFormulaBase(new MathMacroTemplate(nm, na))
{
	MathMacroTable::insertTemplate(tmacro());
}


Inset * InsetFormulaMacro::clone(Buffer const &, bool) const
{
	return new InsetFormulaMacro(*this);
}


void InsetFormulaMacro::write(ostream & os) const
{
	os << "FormulaMacro ";
	tmacro()->write(os, false);
}


int InsetFormulaMacro::latex(ostream & os, bool fragile, 
			     bool /*free_spacing*/) const
{
	tmacro()->write(os, fragile);
	return 2;
}

int InsetFormulaMacro::ascii(ostream & os, int) const
{
	tmacro()->write(os, false);
	return 0;
}


int InsetFormulaMacro::linuxdoc(ostream & os) const
{
	return ascii(os, 0);
}


int InsetFormulaMacro::docBook(ostream & os) const
{
	return ascii(os, 0);
}


void InsetFormulaMacro::read(LyXLex & lex)
{
	// Awful hack...
	par_ = mathed_parse(lex);
	MathMacroTable::insertTemplate(tmacro());
	par_->metrics(LM_ST_TEXT);
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
	tmacro()->metrics(LM_ST_TEXT);
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

	// LColor::mathbg used to be "AntiqueWhite" but is "linen" now, too
	pain.fillRectangle(int(x), y , w, h, LColor::mathmacrobg);
	pain.rectangle(int(x), y, w, h, LColor::mathframe);

	if (mathcursor && mathcursor->formula() == this)
		mathcursor->drawSelection(pain);

	pain.text(int(x + 2), baseline, prefix(), font);
	x += width(bv, font);

	// formula
	float t = tmacro()->width() + 5;
	x -= t;
	tmacro()->draw(pain, int(x), baseline);
	x += t;
}


UpdatableInset::RESULT
InsetFormulaMacro::localDispatch(BufferView * bv,
				 kb_action action, string const & arg)
{
	RESULT result = DISPATCHED;
	switch (action) {
		case LFUN_MATH_MACROARG: {
			int const i = lyx::atoi(arg);
			lyxerr << "inserting macro arg " << i << "\n";
			if (i > 0 && i <= tmacro()->numargs()) {
				mathcursor->insert(new MathMacroArgument(i));
				updateLocal(bv, true);
			} else {
				lyxerr << "not in range 0.." << tmacro()->numargs() << "\n";
			}
			break;
		}
		
		default:
			result = InsetFormulaBase::localDispatch(bv, action, arg);
	}
	return result;
}


MathMacroTemplate * InsetFormulaMacro::tmacro() const
{
	return static_cast<MathMacroTemplate *>(par_);
}


Inset::Code InsetFormulaMacro::lyxCode() const
{
	return Inset::MATHMACRO_CODE;
}


MathInsetTypes InsetFormulaMacro::getType() const
{
	return LM_OT_MACRO;
}
