
#ifdef __GNUG__
#pragma implementation 
#endif

#include <config.h>

#include "math_symbolinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "math_parser.h"
#include "LaTeXFeatures.h"
#include "debug.h"

MathSymbolInset::MathSymbolInset(const latexkeys * l)
	: sym_(l), h_(0)
{}


MathSymbolInset::MathSymbolInset(const char * name)
	: sym_(in_word_set(name)), h_(0)
{}


MathSymbolInset::MathSymbolInset(string const & name)
	: sym_(in_word_set(name.c_str())), h_(0)
{}



MathInset * MathSymbolInset::clone() const
{
	return new MathSymbolInset(*this);
}


string MathSymbolInset::name() const
{
	return sym_->name;
}


void MathSymbolInset::metrics(MathMetricsInfo & mi) const
{
	//lyxerr << "metrics: symbol: '" << sym_->name
	//	<< "' in font: '" << sym_->inset
	//	<< "' drawn as: '" << sym_->draw
	//	<< "'\n";

	MathFontSetChanger dummy(mi.base, sym_->inset.c_str());
	mathed_string_dim(mi.base.font, sym_->draw, dim_);
	// correct height for broken cmex and wasy font
	if (sym_->inset == "cmex" || sym_->inset == "wasy") {
		h_ = 4 * dim_.d / 5;
		dim_.a += h_;
		dim_.d -= h_;
	}
	// seperate things a bit
	int em = mathed_char_width(mi.base.font, 'M');
	if (name() == "not")
		// \not is a special case. 
		// It must have 0 width to align properly with the next symbol.
		dim_.w = 0;
	else if (isRelOp())
		dim_.w += static_cast<int>(0.5*em+0.5);
	else
		dim_.w += static_cast<int>(0.15*em+0.5);

	scriptable_ = false;
	if (mi.base.style == LM_ST_DISPLAY)
		if (sym_->inset == "cmex" || sym_->extra == "funclim")
			scriptable_ = true;
}


void MathSymbolInset::draw(MathPainterInfo & pi, int x, int y) const
{
	//lyxerr << "metrics: symbol: '" << sym_->name
	//	<< "' in font: '" << sym_->inset
	//	<< "' drawn as: '" << sym_->draw
	//	<< "'\n";
	int em = mathed_char_width(pi.base.font, 'M');
	// Here we don't need a special case for \not, as it needs the same
	// increase in x as the next symbol.
	if (isRelOp())
		x += static_cast<int>(0.25*em+0.5);
	else
		x += static_cast<int>(0.075*em+0.5);

	MathFontSetChanger dummy(pi.base, sym_->inset.c_str());
	drawStr(pi, pi.base.font, x, y - h_, sym_->draw);
}


bool MathSymbolInset::isRelOp() const
{
	return sym_->extra == "mathrel";
}


bool MathSymbolInset::isScriptable() const
{
	return scriptable_;
}


bool MathSymbolInset::takesLimits() const
{
	return
		sym_->inset == "cmex" ||
		sym_->inset == "lyxboldsymb" ||
		sym_->extra == "funclim";
}


void MathSymbolInset::validate(LaTeXFeatures & features) const
{
	if (sym_->inset == "msa" || sym_->inset == "msb")
		features.require("amssymb");
}


void MathSymbolInset::normalize(NormalStream & os) const
{
	os << "[symbol " << name() << "]";
}


void MathSymbolInset::maplize(MapleStream & os) const
{
	if (name() == "cdot")
		os << '*';
	else if (name() == "infty")
		os << "infinity";
	else
		os << name();
}

void MathSymbolInset::mathematicize(MathematicaStream & os) const
{
	if ( name() == "pi")    { os << "Pi"; return;}
	if ( name() == "infty") { os << "Infinity"; return;}
	os << name();
}


char const * MathMLtype(string const & s)
{
	if (s == "mathop")
		return "mo";
	return "mi";
}


bool MathSymbolInset::match(MathAtom const & at) const
{
	MathSymbolInset const * q = at->asSymbolInset();
	return q && name() == q->name();
}


void MathSymbolInset::mathmlize(MathMLStream & os) const
{
	char const * type = MathMLtype(sym_->extra);
	os << '<' << type << "> ";
	if (sym_->xmlname == "x") // unknown so far
		os << name();
	else
		os << sym_->xmlname;
	os << " </" << type << '>';
}


void MathSymbolInset::octavize(OctaveStream & os) const
{
	if (name() == "cdot")
		os << '*';
	else
		os << name();
}


void MathSymbolInset::write(WriteStream & os) const
{
	os << '\\' << name();
	os.pendingSpace(true);
}


void MathSymbolInset::infoize(std::ostream & os) const
{
	os << "Symbol: " << name();
}
