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


#include "formulamacro.h"
#include "lfuns.h"
#include "math_cursor.h"
#include "math_parser.h"
#include "math_macro.h"
#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "metricsinfo.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "BufferView.h"
#include "gettext.h"
#include "latexrunparams.h"
#include "frontends/Painter.h"
#include "frontends/font_metrics.h"
#include "support/lyxlib.h"
#include "support/LOstream.h"
#include "debug.h"
#include "lyxlex.h"
#include "lyxtext.h"
#include "lyxfont.h"

#include "Lsstream.h"

#include "support/BoostFormat.h"

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
	MathMacroTable::create(MathAtom(new MathMacroTemplate(name, nargs)));
}


InsetFormulaMacro::InsetFormulaMacro(string const & s)
{
	std::istringstream is(STRCONV(s));
	read(is);
}


Inset * InsetFormulaMacro::clone(Buffer const &) const
{
	return new InsetFormulaMacro(*this);
}


// Inset * InsetFormulaMacro::clone(Buffer const &, bool) const
// {
//	return new InsetFormulaMacro(*this);
// }


void InsetFormulaMacro::write(Buffer const *, ostream & os) const
{
	os << "FormulaMacro ";
	WriteStream wi(os, false, false);
	par()->write(wi);
}


int InsetFormulaMacro::latex(Buffer const *, ostream & os,
			     LatexRunParams const & runparams) const
{
	WriteStream wi(os, runparams.moving_arg, true);
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


int InsetFormulaMacro::docbook(Buffer const * buf, ostream & os, bool) const
{
	return ascii(buf, os, 0);
}


void InsetFormulaMacro::read(Buffer const *, LyXLex & lex)
{
	read(lex.getStream());
}


void InsetFormulaMacro::read(std::istream & is)
{
	MathMacroTemplate * p = new MathMacroTemplate(is);
	setInsetName(p->name());
	MathMacroTable::create(MathAtom(p));
	metrics();
}


string InsetFormulaMacro::prefix() const
{
#if USE_BOOST_FORMAT
	return STRCONV(boost::io::str(boost::format(_(" Macro: %s: ")) %
		STRCONV(getInsetName())));
#else
	return _(" Macro: ") + getInsetName() + ": ";
#endif
}


void InsetFormulaMacro::dimension(BufferView * bv, LyXFont const & font,
	Dimension & dim) const
{
	metrics(bv, font);
	dim = par()->dimensions();
	dim.a += 5;
	dim.d += 5;
	dim.w += 10 + font_metrics::width(prefix(), font);
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


void InsetFormulaMacro::draw(BufferView * bv, LyXFont const & f,
			     int y, float & xx) const
{
	// label
	LyXFont font(f);
	font.setColor(LColor::math);

	PainterInfo pi = PainterInfo(bv->painter());
	pi.base.style = LM_ST_TEXT;
	pi.base.font  = font;

	Dimension dim;
	dimension(bv, font, dim);
	int const x = int(xx);
	int const a = y - dim.a + 1;
	int const w = dim.w - 2;
	int const h = dim.height() - 2;

	// LColor::mathbg used to be "AntiqueWhite" but is "linen" now, too
	pi.pain.fillRectangle(x, a, w, h, LColor::mathmacrobg);
	pi.pain.rectangle(x, a, w, h, LColor::mathframe);

	if (mathcursor &&
			const_cast<InsetFormulaBase const *>(mathcursor->formula()) == this)
		mathcursor->drawSelection(pi);

	pi.pain.text(x + 2, y, prefix(), font);

	// formula
	par()->draw(pi, x + font_metrics::width(prefix(), f) + 5, y);
	xx += w + 2;
	xo_ = x;
	yo_ = y;
}
