/**
 * \file math_macrotemplate.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_macrotemplate.h"
#include "math_mathmlstream.h"
#include "math_parser.h"
#include "math_support.h"

#include "cursor.h"
#include "debug.h"
#include "gettext.h"
#include "lyxlex.h"
#include "LColor.h"

#include "frontends/Painter.h"
#include "frontends/font_metrics.h"

#include "support/lstrings.h"

using lyx::support::bformat;

using std::string;
using std::auto_ptr;
using std::ostream;
using std::endl;


MathMacroTemplate::MathMacroTemplate()
	: MathNestInset(2), numargs_(0), name_(), type_("newcommand")
{}


MathMacroTemplate::MathMacroTemplate(string const & nm, int numargs,
		string const & type, MathArray const & ar1, MathArray const & ar2)
	: MathNestInset(2), numargs_(numargs), name_(nm), type_(type)
{
	if (numargs_ > 9)
		lyxerr << "MathMacroTemplate::MathMacroTemplate: wrong # of arguments: "
			<< numargs_ << std::endl;
	cell(0) = ar1;
	cell(1) = ar2;
}


MathMacroTemplate::MathMacroTemplate(std::istream & is)
	: MathNestInset(2), numargs_(0), name_()
{
	MathArray ar;
	mathed_parse_cell(ar, is);
	if (ar.size() != 1 || !ar[0]->asMacroTemplate()) {
		lyxerr << "cannot read macro from '" << ar << "'" << endl;
		return;
	}
	operator=( *(ar[0]->asMacroTemplate()) );
}


auto_ptr<InsetBase> MathMacroTemplate::clone() const
{
	return auto_ptr<InsetBase>(new MathMacroTemplate(*this));
}


void MathMacroTemplate::edit(LCursor & cur, bool left)
{
	lyxerr << "MathMacroTemplate: edit left/right" << endl;
	cur.push(*this);
}


int MathMacroTemplate::numargs() const
{
	return numargs_;
}


void MathMacroTemplate::numargs(int numargs)
{
	numargs_ = numargs;
}


string MathMacroTemplate::name() const
{
	return name_;
}


string MathMacroTemplate::prefix() const
{
	return bformat(_(" Macro: %1$s: "), name_);
}


void MathMacroTemplate::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi);
	cell(1).metrics(mi);
	dim.wid = cell(0).width() + cell(1).width() + 20
		+ font_metrics::width(prefix(), mi.base.font);
	dim.asc = std::max(cell(0).ascent(),  cell(1).ascent())  + 7;
	dim.des = std::max(cell(0).descent(), cell(1).descent()) + 7;
	dim_ = dim;
}


void MathMacroTemplate::draw(PainterInfo & p, int x, int y) const
{
	setPosCache(p, x, y);

	// label
	LyXFont font = p.base.font;
	font.setColor(LColor::math);

	PainterInfo pi(p.base.bv, p.pain);
	pi.base.style = LM_ST_TEXT;
	pi.base.font  = font;

	int const a = y - dim_.asc + 1;
	int const w = dim_.wid - 2;
	int const h = dim_.height() - 2;

	// LColor::mathbg used to be "AntiqueWhite" but is "linen" now, too
	// the next line would overwrite the selection!
	//pi.pain.fillRectangle(x, a, w, h, LColor::mathmacrobg);
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
	x += font_metrics::width(prefix(), pi.base.font) + 6;

	int const w0 = cell(0).width();
	int const w1 = cell(1).width();
	cell(0).draw(pi, x + 2, y + 1);
	pi.pain.rectangle(x, y - dim_.ascent() + 3,
		w0 + 4, dim_.height() - 6, LColor::mathline);
	cell(1).draw(pi, x + 8 + w0, y + 1);
	pi.pain.rectangle(x + w0 + 6, y - dim_.ascent() + 3,
		w1 + 4, dim_.height() - 6, LColor::mathline);
}


void MathMacroTemplate::read(Buffer const &, LyXLex & lex)
{
	MathArray ar;
	mathed_parse_cell(ar, lex.getStream());
	if (ar.size() != 1 || !ar[0]->asMacroTemplate()) {
		lyxerr << "cannot read macro from '" << ar << "'" << endl;
		return;
	}
	operator=( *(ar[0]->asMacroTemplate()) );
}


void MathMacroTemplate::write(Buffer const &, std::ostream & os) const
{
	WriteStream wi(os, false, false);
	os << "FormulaMacro ";
	write(wi);
}


void MathMacroTemplate::write(WriteStream & os) const
{
	if (type_ == "def") {
		os << "\n\\def\\" << name_.c_str();
		for (int i = 1; i <= numargs_; ++i)
			os << '#' << i;
	} else {
		// newcommand or renewcommand
		os << "\n\\" << type_.c_str() << "{\\" << name_.c_str() << '}';
		if (numargs_ > 0)
			os << '[' << numargs_ << ']';
	}

	os << '{' << cell(0) << "}";

	if (os.latex()) {
		// writing .tex. done.
		os << "\n";
	} else {
		// writing .lyx, write special .tex export only if necessary
		if (!cell(1).empty())
			os << "\n{" << cell(1) << '}';
	}
}


MacroData MathMacroTemplate::asMacroData() const
{
	return MacroData(asString(cell(0)), numargs(), asString(cell(1)));
}
