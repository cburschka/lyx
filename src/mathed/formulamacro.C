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
	: tmacro_(new MathMacroTemplate("unknown", 0))
{}


InsetFormulaMacro::InsetFormulaMacro(string nm, int na)
	: tmacro_(new MathMacroTemplate(nm, na))
{
	MathMacroTable::insertTemplate(tmacro_);
}


InsetFormulaMacro::~InsetFormulaMacro()
{
#ifdef WITH_WARNINGS
#warning Need to unregister from MathMacroTable.
#endif
	// Instead of unregister an delete leak this until it gets fixed
	//delete tmacro_;
}


Inset * InsetFormulaMacro::clone(Buffer const &, bool) const
{
#ifdef WITH_WARNINGS
#warning This should not be needed in reality...
#endif
	return new InsetFormulaMacro(*this);
}


void InsetFormulaMacro::write(ostream & os) const
{
	os << "FormulaMacro ";
	tmacro().write(os, false);
}


int InsetFormulaMacro::latex(ostream & os, bool fragile, 
			     bool /*free_spacing*/) const
{
	tmacro().write(os, fragile);
	return 2;
}

int InsetFormulaMacro::ascii(ostream & os, int) const
{
	tmacro().write(os, false);
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
	delete tmacro_;
	tmacro_ = mathed_parse_macro(lex);
	MathMacroTable::insertTemplate(tmacro_);
	metrics();
}


string InsetFormulaMacro::prefix() const
{
	return string(" ") + _("Macro: ") + tmacro().name() + ": ";
}


int InsetFormulaMacro::ascent(BufferView *, LyXFont const &) const
{
	return tmacro().ascent() + 5;
}


int InsetFormulaMacro::descent(BufferView *, LyXFont const &) const
{
	return tmacro().descent() + 5;
}


int InsetFormulaMacro::width(BufferView *, LyXFont const & f) const
{
	metrics();
	return 10 + lyxfont::width(prefix(), f) + tmacro().width();
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
			if (i > 0 && i <= tmacro().numargs()) {
				mathcursor->insert(new MathMacroArgument(i));
				updateLocal(bv, true);
			} else {
				lyxerr << "not in range 0.." << tmacro().numargs() << "\n";
			}
			break;
		}
		
		default:
			result = InsetFormulaBase::localDispatch(bv, action, arg);
	}
	return result;
}


MathMacroTemplate const & InsetFormulaMacro::tmacro() const
{
	return *tmacro_;
}


Inset::Code InsetFormulaMacro::lyxCode() const
{
	return Inset::MATHMACRO_CODE;
}


MathInsetTypes InsetFormulaMacro::getType() const
{
	return LM_OT_MACRO;
}


MathInset * InsetFormulaMacro::par() const
{
	return const_cast<MathMacroTemplate *>(tmacro_);
}


void InsetFormulaMacro::metrics() const
{
	par()->metrics(LM_ST_TEXT);
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
	float t = tmacro().width() + 5;
	x -= t;
	par()->draw(pain, int(x), baseline);
	x += t;
}

