/**
 * \file InsetMathSymbol.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathSymbol.h"

#include "MathAtom.h"
#include "MathParser.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "Dimension.h"
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/lyxlib.h"
#include "support/textutils.h"
#include "support/unique_ptr.h"


namespace lyx {

InsetMathSymbol::InsetMathSymbol(latexkeys const * l)
	: sym_(l), h_(0), kerning_(0), scriptable_(false)
{}


InsetMathSymbol::InsetMathSymbol(char const * name)
	: sym_(in_word_set(from_ascii(name))), h_(0),
	  kerning_(0), scriptable_(false)
{}


InsetMathSymbol::InsetMathSymbol(docstring const & name)
	: sym_(in_word_set(name)), h_(0), kerning_(0), scriptable_(false)
{}


Inset * InsetMathSymbol::clone() const
{
	return new InsetMathSymbol(*this);
}


docstring InsetMathSymbol::name() const
{
	return sym_->name;
}


void InsetMathSymbol::metrics(MetricsInfo & mi, Dimension & dim) const
{
	// set dim and negative kerning_ to move a subscript leftward
	kerning_ = -mathedSymbolDim(mi.base, dim, sym_);
	// correct height for broken cmex and wasy font
	if (sym_->inset == "cmex" || sym_->inset == "wasy") {
		h_ = 4 * dim.des / 5;
		dim.asc += h_;
		dim.des -= h_;
	}
	// set scriptable_
	scriptable_ = false;
	if (mi.base.font.style() == LM_ST_DISPLAY)
		if (sym_->inset == "cmex" || sym_->inset == "esint" ||
		    sym_->extra == "funclim" ||
		    (sym_->inset == "stmry" && sym_->extra == "mathop"))
			scriptable_ = true;
}


void InsetMathSymbol::draw(PainterInfo & pi, int x, int y) const
{
	mathedSymbolDraw(pi, x, y - h_, sym_);
}


InsetMath::mode_type InsetMathSymbol::currentMode() const
{
	return sym_->extra == "textmode" ? TEXT_MODE : MATH_MODE;
}


bool InsetMathSymbol::isOrdAlpha() const
{
	return sym_->extra == "mathord" || sym_->extra == "mathalpha";
}


MathClass InsetMathSymbol::mathClass() const
{
	if (sym_->extra == "func" || sym_->extra == "funclim")
		return MC_OP;
	MathClass const mc = string_to_class(sym_->extra);
	return (mc == MC_UNKNOWN) ? MC_ORD : mc;
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
		sym_->inset == "esint" ||
		sym_->extra == "funclim" ||
		(sym_->inset == "stmry" && sym_->extra == "mathop");
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


void InsetMathSymbol::mathmlize(MathStream & os) const
{
	// FIXME We may need to do more interesting things
	// with MathMLtype.
	char const * type = sym_->MathMLtype();
	os << '<' << type << "> ";
	if (sym_->xmlname == "x")
		// unknown so far
		os << name();
	else
		os << sym_->xmlname;
	os << " </" << type << '>';
}


void InsetMathSymbol::htmlize(HtmlStream & os, bool spacing) const
{
	// FIXME We may need to do more interesting things
	// with MathMLtype.
	char const * type = sym_->MathMLtype();
	bool op = (std::string(type) == "mo");

	if (sym_->xmlname == "x")
		// unknown so far
		os << ' ' << name() << ' ';
	else if (op && spacing)
		os << ' ' << sym_->xmlname << ' ';
	else
		os << sym_->xmlname;
}


void InsetMathSymbol::htmlize(HtmlStream & os) const
{
	htmlize(os, true);
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
	unique_ptr<MathEnsurer> ensurer;
	if (currentMode() != TEXT_MODE)
		ensurer = make_unique<MathEnsurer>(os);
	else
		ensurer = make_unique<MathEnsurer>(os, false, true, true);
	os << '\\' << name();

	// $,#, etc. In theory the restriction based on catcodes, but then
	// we do not handle catcodes very well, let alone cat code changes,
	// so being outside the alpha range is good enough.
	if (name().size() == 1 && !isAlphaASCII(name()[0]))
		return;

	os.pendingSpace(true);
}


void InsetMathSymbol::infoize2(odocstream & os) const
{
	os << from_ascii("Symbol: ") << name();
}


void InsetMathSymbol::validate(LaTeXFeatures & features) const
{
	// this is not really the ideal place to do this, but we can't
	// validate in InsetMathExInt.
	if (features.runparams().math_flavor == OutputParams::MathAsHTML
	    && sym_->name == from_ascii("int")) {
		features.addCSSSnippet(
			"span.limits{display: inline-block; vertical-align: middle; text-align:center; font-size: 75%;}\n"
			"span.limits span{display: block;}\n"
			"span.intsym{font-size: 150%;}\n"
			"sub.limit{font-size: 75%;}\n"
			"sup.limit{font-size: 75%;}");
	} else {
		if (!sym_->requires.empty())
			features.require(sym_->requires);
	}
}

} // namespace lyx
