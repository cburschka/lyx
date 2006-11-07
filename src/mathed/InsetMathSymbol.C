/**
 * \file InsetMathSymbol.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathSymbol.h"
#include "dimension.h"
#include "MathStream.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "MathParser.h"
#include "MathAtom.h"
#include "LaTeXFeatures.h"
#include "debug.h"


namespace lyx {

using std::string;
using std::auto_ptr;


InsetMathSymbol::InsetMathSymbol(latexkeys const * l)
	: sym_(l), h_(0), width_(0), scriptable_(false)
{}


InsetMathSymbol::InsetMathSymbol(char const * name)
	: sym_(in_word_set(from_ascii(name))), h_(0), width_(0), scriptable_(false)
{}


InsetMathSymbol::InsetMathSymbol(docstring const & name)
	: sym_(in_word_set(name)), h_(0), width_(0), scriptable_(false)
{}


auto_ptr<InsetBase> InsetMathSymbol::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathSymbol(*this));
}


docstring InsetMathSymbol::name() const
{
	return sym_->name;
}


void InsetMathSymbol::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//lyxerr << "metrics: symbol: '" << sym_->name
	//	<< "' in font: '" << sym_->inset
	//	<< "' drawn as: '" << sym_->draw
	//	<< "'" << std::endl;

	int const em = mathed_char_width(mi.base.font, 'M');
	FontSetChanger dummy(mi.base, sym_->inset);
	mathed_string_dim(mi.base.font, sym_->draw, dim);
	// correct height for broken cmex and wasy font
#if defined(__APPLE__) && defined(__GNUC__)
	if (sym_->inset == "cmex") {
		h_ = 4 * dim.des / 5;
		dim.asc += 0*h_;
		dim.des -= h_;
		h_ = dim.asc;
	} else if (sym_->inset == "wasy") {
		h_ = 4 * dim.des / 5;
		dim.asc += h_;
		dim.des -= h_;
	}
#else
	if (sym_->inset == "cmex" || sym_->inset == "wasy") {
		h_ = 4 * dim.des / 5;
		dim.asc += h_;
		dim.des -= h_;
	}
#endif
	// seperate things a bit
	if (isRelOp())
		dim.wid += static_cast<int>(0.5 * em + 0.5);
	else
		dim.wid += static_cast<int>(0.1667 * em + 0.5);

	scriptable_ = false;
	if (mi.base.style == LM_ST_DISPLAY)
		if (sym_->inset == "cmex" || sym_->extra == "funclim")
			scriptable_ = true;

	width_ = dim.wid;
}


void InsetMathSymbol::draw(PainterInfo & pi, int x, int y) const
{
	//lyxerr << "metrics: symbol: '" << sym_->name
	//	<< "' in font: '" << sym_->inset
	//	<< "' drawn as: '" << sym_->draw
	//	<< "'" << std::endl;
	int const em = mathed_char_width(pi.base.font, 'M');
	if (isRelOp())
		x += static_cast<int>(0.25*em+0.5);
	else
		x += static_cast<int>(0.0833*em+0.5);

	FontSetChanger dummy(pi.base, sym_->inset.c_str());
	pi.draw(x, y - h_, sym_->draw);
}


bool InsetMathSymbol::isRelOp() const
{
	return sym_->extra == "mathrel";
}


bool InsetMathSymbol::isScriptable() const
{
	return scriptable_;
}


bool InsetMathSymbol::takesLimits() const
{
	return
		sym_->inset == "cmex" ||
		sym_->inset == "lyxboldsymb" ||
		sym_->extra == "funclim";
}


void InsetMathSymbol::validate(LaTeXFeatures & features) const
{
	if (!sym_->requires.empty())
		features.require(to_utf8(sym_->requires));
}


void InsetMathSymbol::normalize(NormalStream & os) const
{
	os << "[symbol " << name() << ']';
}


void InsetMathSymbol::maple(MapleStream & os) const
{
	if (name() == "cdot")
		os << '*';
	else if (name() == "infty")
		os << "infinity";
	else
		os << name();
}

void InsetMathSymbol::maxima(MaximaStream & os) const
{
	if (name() == "cdot")
		os << '*';
	else if (name() == "infty")
		os << "inf";
	else if (name() == "pi")
		os << "%pi";
	else
		os << name();
}


void InsetMathSymbol::mathematica(MathematicaStream & os) const
{
	if ( name() == "pi")    { os << "Pi"; return;}
	if ( name() == "infty") { os << "Infinity"; return;}
	if ( name() == "cdot")  { os << '*'; return;}
	os << name();
}


char const * MathMLtype(docstring const & s)
{
	if (s == "mathop")
		return "mo";
	return "mi";
}


void InsetMathSymbol::mathmlize(MathStream & os) const
{
	char const * type = MathMLtype(sym_->extra);
	os << '<' << type << "> ";
	if (sym_->xmlname == "x") // unknown so far
		os << name();
	else
		os << sym_->xmlname;
	os << " </" << type << '>';
}


void InsetMathSymbol::octave(OctaveStream & os) const
{
	if (name() == "cdot")
		os << '*';
	else
		os << name();
}


void InsetMathSymbol::write(WriteStream & os) const
{
	os << '\\' << name();
	os.pendingSpace(true);
}


void InsetMathSymbol::infoize2(odocstream & os) const
{
	os << "Symbol: " << name();
}


} // namespace lyx
