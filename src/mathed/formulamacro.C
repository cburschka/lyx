/**
 * \file formulamacro.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "formulamacro.h"
#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "math_mathmlstream.h"

#include "BufferView.h"
#include "cursor.h"
#include "debug.h"
#include "gettext.h"
#include "LColor.h"
#include "lyxlex.h"
#include "outputparams.h"

#include "frontends/Painter.h"
#include "frontends/font_metrics.h"

#include "support/lstrings.h"
#include "support/std_sstream.h"

using lyx::support::bformat;

using std::string;
using std::auto_ptr;
using std::ostream;
using std::endl;



InsetFormulaMacro::InsetFormulaMacro()
	: MathNestInset(2), name_("unknownA")
{}


InsetFormulaMacro::InsetFormulaMacro
		(string const & name, int nargs, string const & type)
	: MathNestInset(2), name_(name)
{
	MathMacroTable::create(MathAtom(new MathMacroTemplate(name, nargs, type)));
}


InsetFormulaMacro::InsetFormulaMacro(string const & s)
	: MathNestInset(2), name_("unknownB")
{
	std::istringstream is(s);
	read(is);
}


auto_ptr<InsetBase> InsetFormulaMacro::clone() const
{
	return auto_ptr<InsetBase>(new InsetFormulaMacro(*this));
}


void InsetFormulaMacro::write(Buffer const &, ostream & os) const
{
	os << "FormulaMacro ";
	WriteStream wi(os, false, false);
	tmpl()->write(wi);
}


int InsetFormulaMacro::latex(Buffer const &, ostream & os,
			     OutputParams const & runparams) const
{
	//lyxerr << "InsetFormulaMacro::latex" << endl;
	WriteStream wi(os, runparams.moving_arg, true);
	tmpl()->write(wi);
	return 2;
}


int InsetFormulaMacro::plaintext(Buffer const &, ostream & os,
			     OutputParams const &) const
{
	WriteStream wi(os, false, true);
	tmpl()->write(wi);
	return 0;
}


int InsetFormulaMacro::linuxdoc(Buffer const & buf, ostream & os,
				OutputParams const & runparams) const
{
	return plaintext(buf, os, runparams);
}


int InsetFormulaMacro::docbook(Buffer const & buf, ostream & os,
			       OutputParams const & runparams) const
{
	return plaintext(buf, os, runparams);
}


void InsetFormulaMacro::read(Buffer const &, LyXLex & lex)
{
	read(lex.getStream());
}


void InsetFormulaMacro::read(std::istream & is)
{
	auto_ptr<MathMacroTemplate> p(new MathMacroTemplate(is));
	name_ = p->name();
	MathMacroTable::create(MathAtom(p.release()));
}


string InsetFormulaMacro::prefix() const
{
	return bformat(_(" Macro: %1$s: "), name_);
}


void InsetFormulaMacro::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//lyxerr << "InsetFormulaMacro: " << this << " -- " << &tmpl() << endl;
	tmpl()->metrics(mi, dim);
	dim.asc += 5;
	dim.des += 5;
	dim.wid += 10 + font_metrics::width(prefix(), mi.base.font);
	dim_ = dim;
}


void InsetFormulaMacro::draw(PainterInfo & p, int x, int y) const
{
	// label
	LyXFont font = p.base.font;
	font.setColor(LColor::math);

	PainterInfo pi(p.base.bv);
	pi.base.style = LM_ST_TEXT;
	pi.base.font  = font;

	int const a = y - dim_.asc + 1;
	int const w = dim_.wid - 2;
	int const h = dim_.height() - 2;

	// LColor::mathbg used to be "AntiqueWhite" but is "linen" now, too
	pi.pain.fillRectangle(x, a, w, h, LColor::mathmacrobg);
	pi.pain.rectangle(x, a, w, h, LColor::mathframe);

#ifdef WITH_WARNINGS
#warning FIXME
#endif
#if 0
	LCursor & cur = p.base.bv->cursor();
	if (cur.isInside(this))
		cur.drawSelection(pi);
#endif

	pi.pain.text(x + 2, y, prefix(), font);

	// body
	tmpl()->draw(pi, x + font_metrics::width(prefix(), p.base.font) + 5, y);

	setPosCache(pi, x, y);
}


MathAtom & InsetFormulaMacro::tmpl() const
{
	return MathMacroTable::provide(name_);
}
