#include <config.h>

#include "math_symbolinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "math_parser.h"
#include "LaTeXFeatures.h"
#include "debug.h"


using std::ostream;


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
	mathed_string_dim(mi.base.font, sym_->draw, ascent_, descent_, width_);
	// correct height for broken cmex font
	if (sym_->inset == "cmex") {
		h_ = 4 * descent_ / 5;
		ascent_  += h_;
		descent_ -= h_;
	}
	if (isRelOp())
		width_ += 6;

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
	if (isRelOp())
		x += 3;
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
	else
		os << name();
}


char const * MathMLtype(string const & s)
{
	if (s == "mathop")
		return "mo";
	return "mi";
}


bool MathSymbolInset::match(MathInset * p) const
{
	MathSymbolInset const * q = p->asSymbolInset();
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
	os << '\\' << name() << ' ';
}


void MathSymbolInset::infoize(ostream & os) const
{
	os << '\\' << name();
}
