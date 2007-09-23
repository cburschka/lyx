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

#include "MathMacroTemplate.h"
#include "MathStream.h"
#include "MathParser.h"
#include "MathSupport.h"

#include "Buffer.h"
#include "Cursor.h"
#include "debug.h"
#include "gettext.h"
#include "Lexer.h"
#include "Color.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/lstrings.h"


namespace lyx {

using support::bformat;

using std::ostream;
using std::endl;


MathMacroTemplate::MathMacroTemplate()
	: InsetMathNest(2), numargs_(0), name_(), type_(from_ascii("newcommand"))
{
	initMath();
}


MathMacroTemplate::MathMacroTemplate(docstring const & name, int numargs,
		docstring const & type, MathData const & ar1, MathData const & ar2)
	: InsetMathNest(2), numargs_(numargs), name_(name), type_(type)
{
	initMath();

	if (numargs_ > 9)
		lyxerr << "MathMacroTemplate::MathMacroTemplate: wrong # of arguments: "
			<< numargs_ << std::endl;
	cell(0) = ar1;
	cell(1) = ar2;
}


MathMacroTemplate::MathMacroTemplate(docstring const & str)
	: InsetMathNest(2), numargs_(0), name_()
{
	initMath();

	MathData ar;
	mathed_parse_cell(ar, str);
	if (ar.size() != 1 || !ar[0]->asMacroTemplate()) {
		lyxerr << "Cannot read macro from '" << ar << "'" << endl;
		return;
	}
	operator=( *(ar[0]->asMacroTemplate()) );
}


Inset * MathMacroTemplate::clone() const
{
	return new MathMacroTemplate(*this);
}


void MathMacroTemplate::edit(Cursor & cur, bool)
{
	//lyxerr << "MathMacroTemplate: edit left/right" << endl;
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


docstring MathMacroTemplate::name() const
{
	return name_;
}


docstring MathMacroTemplate::prefix() const
{
	return bformat(_(" Macro: %1$s: "), name_);
}


void MathMacroTemplate::metrics(MetricsInfo & mi, Dimension & dim) const
{
	bool lockMacro = MacroTable::globalMacros().has(name_);
	if (lockMacro)
		MacroTable::globalMacros().get(name_).lock();

	cell(0).metrics(mi);
	cell(1).metrics(mi);
	docstring dp = prefix();
	dim.wid = cell(0).width() + cell(1).width() + 20
		+ theFontMetrics(mi.base.font).width(dp);
	dim.asc = std::max(cell(0).ascent(),  cell(1).ascent())  + 7;
	dim.des = std::max(cell(0).descent(), cell(1).descent()) + 7;

	if (lockMacro)
		MacroTable::globalMacros().get(name_).unlock();

	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void MathMacroTemplate::draw(PainterInfo & p, int x, int y) const
{
	bool lockMacro = MacroTable::globalMacros().has(name_);
	if (lockMacro)
		MacroTable::globalMacros().get(name_).lock();

	setPosCache(p, x, y);

	Dimension const dim = dimension(*p.base.bv);

	// label
	Font font = p.base.font;
	font.setColor(Color::math);

	PainterInfo pi(p.base.bv, p.pain);
	pi.base.style = LM_ST_TEXT;
	pi.base.font  = font;

	int const a = y - dim.asc + 1;
	int const w = dim.wid - 2;
	int const h = dim.height() - 2;

	// Color::mathbg used to be "AntiqueWhite" but is "linen" now, too
	// the next line would overwrite the selection!
	//pi.pain.fillRectangle(x, a, w, h, Color::mathmacrobg);
	pi.pain.rectangle(x, a, w, h, Color::mathframe);

	// FIXME:
#if 0
	Cursor & cur = p.base.bv->cursor();
	if (cur.isInside(this))
		cur.drawSelection(pi);
#endif
	docstring dp = prefix();
	pi.pain.text(x + 2, y, dp, font);
	// FIXME: Painter text should retain the drawn text width
	x += theFontMetrics(font).width(dp) + 6;

	int const w0 = cell(0).width();
	int const w1 = cell(1).width();
	cell(0).draw(pi, x + 2, y + 1);
	pi.pain.rectangle(x, y - dim.ascent() + 3,
		w0 + 4, dim.height() - 6, Color::mathline);
	cell(1).draw(pi, x + 8 + w0, y + 1);
	pi.pain.rectangle(x + w0 + 6, y - dim.ascent() + 3,
		w1 + 4, dim.height() - 6, Color::mathline);

	if (lockMacro)
		MacroTable::globalMacros().get(name_).unlock();
}


void MathMacroTemplate::read(Buffer const &, Lexer & lex)
{
	MathData ar;
	mathed_parse_cell(ar, lex.getStream());
	if (ar.size() != 1 || !ar[0]->asMacroTemplate()) {
		lyxerr << "Cannot read macro from '" << ar << "'" << endl;
		lyxerr << "Read: " << to_utf8(asString(ar)) << endl;
		return;
	}
	operator=( *(ar[0]->asMacroTemplate()) );
}


void MathMacroTemplate::write(Buffer const &, std::ostream & os) const
{
	odocstringstream oss;
	WriteStream wi(oss, false, false);
	oss << "FormulaMacro\n";
	write(wi);
	os << to_utf8(oss.str());
}


void MathMacroTemplate::write(WriteStream & os) const
{
	if (type_ == "def") {
		os << "\\def\\" << name_.c_str();
		for (int i = 1; i <= numargs_; ++i)
			os << '#' << i;
	} else {
		// newcommand or renewcommand
		os << "\\" << type_.c_str() << "{\\" << name_.c_str() << '}';
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


int MathMacroTemplate::plaintext(Buffer const & buf, odocstream & os,
				 OutputParams const &) const
{
	static docstring const str = '[' + buf.B_("math macro") + ']';

	os << str;
	return str.size();
}


MacroData MathMacroTemplate::asMacroData() const
{
	return MacroData(asString(cell(0)), numargs(), asString(cell(1)), std::string());
}


} // namespace lyx
