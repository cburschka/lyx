/*
 *  File:        formulamacro.C
 *  Purpose:     Implementation of the formula macro LyX inset
 *  Author:      André Pönitz, based on ideas of Alejandro Aguilar Sierra
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
#include "math_macrotable.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "BufferView.h"
#include "gettext.h"
#include "Painter.h"
#include "font.h"
#include "support/lyxlib.h"
#include "support/LOstream.h"
#include "debug.h"
#include "lyxlex.h"
#include "lyxtext.h"
#include "lyxfont.h"

using std::ostream;

extern MathCursor * mathcursor;

InsetFormulaMacro::InsetFormulaMacro()
{
	// inset name is inherited from Inset
	setInsetName("unknown");
}


InsetFormulaMacro::InsetFormulaMacro(string const & name, int nargs)
{
	setInsetName(name);
	MathMacroTable::create(name, nargs);
}


InsetFormulaMacro::InsetFormulaMacro(string const & s)
{
	string name;
	mathed_parse_macro(name, s);
	setInsetName(name);
}


Inset * InsetFormulaMacro::clone(Buffer const &, bool) const
{
	return new InsetFormulaMacro(*this);
}


void InsetFormulaMacro::write(Buffer const *, ostream & os) const
{
	os << "FormulaMacro ";
	WriteStream wi(os, false, false);
	par()->write(wi);
}


int InsetFormulaMacro::latex(Buffer const *, ostream & os, bool fragile,
			     bool /*free_spacing*/) const
{
	WriteStream wi(os, fragile, true);
	par()->write(wi);
	return 2;
}


int InsetFormulaMacro::ascii(Buffer const *, ostream & os, int) const
{
	WriteStream wi(os, false, true);
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
	string name;
	mathed_parse_macro(name, lex);
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
			     int y, float & xx, bool /*cleared*/) const
{
	Painter & pain = bv->painter();
	LyXFont font(f);

	// label
	font.setColor(LColor::math);

	int const x = int(xx);
	int const a = y - ascent(bv, font) + 1;
	int const w = width(bv, font) - 2;
	int const h = ascent(bv, font) + descent(bv, font) - 2;

	// LColor::mathbg used to be "AntiqueWhite" but is "linen" now, too
	pain.fillRectangle(x, a, w, h, LColor::mathmacrobg);
	pain.rectangle(x, a, w, h, LColor::mathframe);

	if (mathcursor &&
			const_cast<InsetFormulaBase const *>(mathcursor->formula()) == this)
		mathcursor->drawSelection(pain);

	pain.text(x + 2, y, prefix(), font);

	// formula
	par()->draw(pain, x + lyxfont::width(prefix(), f) + 5, y);
	xx += w + 2;
	xo_ = x;
	yo_ = y;

	setCursorVisible(false);
}
