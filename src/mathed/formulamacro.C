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
#include "mathed/math_support.h"
#include "support/LOstream.h"
#include "debug.h"
#include "lyxlex.h"
#include "lyxtext.h"
#include "lyxfont.h"
#include "math_mathmlstream.h"

using std::ostream;

extern MathCursor * mathcursor;

InsetFormulaMacro::InsetFormulaMacro()
{
	// inset name is inherited from Inset
	setInsetName("unknown");
}


InsetFormulaMacro::InsetFormulaMacro(string nm, int na)
{
	setInsetName(nm);
	MathMacroTable::create(nm, na, string());
}


InsetFormulaMacro::InsetFormulaMacro(string const & s)
{
	string name = mathed_parse_macro(s);
	setInsetName(name);
#ifdef WITH_WARNINGS
#warning "metrics disabled"
#endif
	//metrics();
}


Inset * InsetFormulaMacro::clone(Buffer const &, bool) const
{
	return new InsetFormulaMacro(*this);
}


void InsetFormulaMacro::write(Buffer const * buf, ostream & os) const
{
	os << "FormulaMacro ";
	MathWriteInfo wi(buf, os, false);
	par()->write(wi);
}


int InsetFormulaMacro::latex(Buffer const * buf, ostream & os, bool fragile, 
			     bool /*free_spacing*/) const
{
	MathWriteInfo wi(buf, os, fragile);
	par()->write(wi);
	return 2;
}


int InsetFormulaMacro::ascii(Buffer const * buf, ostream & os, int) const
{
	MathWriteInfo wi(buf, os, false);
	par()->write(wi);
	return 0;
}


int InsetFormulaMacro::linuxdoc(Buffer const * buf, ostream & os) const
{
	return ascii(buf, os, 0);
}


int InsetFormulaMacro::docbook(Buffer const * buf, ostream & os) const
{
	return ascii(buf, os, 0);
}


void InsetFormulaMacro::read(Buffer const *, LyXLex & lex)
{
	string name = mathed_parse_macro(lex);
	setInsetName(name);
	//lyxerr << "metrics disabled";
	metrics();
}


string InsetFormulaMacro::prefix() const
{
	return string(" ") + _("Macro: ") + getInsetName() + ": ";
}


int InsetFormulaMacro::ascent(BufferView *, LyXFont const &) const
{
	return par()->ascent() + 5;
}


int InsetFormulaMacro::descent(BufferView *, LyXFont const &) const
{
	return par()->descent() + 5;
}


int InsetFormulaMacro::width(BufferView * bv, LyXFont const & f) const
{
	metrics(bv, f);
	return 10 + lyxfont::width(prefix(), f) + par()->width();
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
			//if (i > 0 && i <= par()->numargs()) {
				mathcursor->insert(MathAtom(new MathMacroArgument(i)));
				updateLocal(bv, true);
			//} else {
			//	lyxerr << "not in range 0.." << par()->numargs() << "\n";
			//}
			break;
		}
		
		default: {
			result = InsetFormulaBase::localDispatch(bv, action, arg);
			// force redraw if anything happened
			if (result != UNDISPATCHED) {
				bv->text->status(bv, LyXText::NEED_MORE_REFRESH);
				bv->updateInset(this, false);
			}
		}
	}
	return result;
}


MathAtom const & InsetFormulaMacro::par() const
{
	return MathMacroTable::provide(getInsetName());
}


MathAtom & InsetFormulaMacro::par()
{
	return MathMacroTable::provide(getInsetName());
}


Inset::Code InsetFormulaMacro::lyxCode() const
{
	return Inset::MATHMACRO_CODE;
}


MathInsetTypes InsetFormulaMacro::getType() const
{
	return LM_OT_MACRO;
}


void InsetFormulaMacro::draw(BufferView * bv, LyXFont const & f,
			     int y, float & x, bool /*cleared*/) const
{
	Painter & pain = bv->painter();
	LyXFont font(f);

	// label
	font.setColor(LColor::math);
	
	int const a = y - ascent(bv, font) + 1;
	int const w = width(bv, font) - 2;
	int const h = ascent(bv, font) + descent(bv, font) - 2;

	// LColor::mathbg used to be "AntiqueWhite" but is "linen" now, too
	pain.fillRectangle(int(x), a , w, h, LColor::mathmacrobg);
	pain.rectangle(int(x), a, w, h, LColor::mathframe);

	if (mathcursor && mathcursor->formula() == this)
		mathcursor->drawSelection(pain);

	pain.text(int(x + 2), y, prefix(), font);
	x += width(bv, font);

	// formula
	xo_ = int(x) - par()->width() - 5;
	yo_ = y;
	par()->draw(pain, xo_, yo_);
}

