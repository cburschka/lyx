/**
 * \file math_symbolinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_symbolinset.h"
#include "dimension.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "math_parser.h"
#include "math_atom.h"
#include "LaTeXFeatures.h"
#include "debug.h"


using std::string;
using std::auto_ptr;


MathSymbolInset::MathSymbolInset(latexkeys const * l)
	: sym_(l), h_(0)
{}


MathSymbolInset::MathSymbolInset(char const * name)
	: sym_(in_word_set(name)), h_(0)
{}


MathSymbolInset::MathSymbolInset(string const & name)
	: sym_(in_word_set(name.c_str())), h_(0)
{}


auto_ptr<InsetBase> MathSymbolInset::clone() const
{
	return auto_ptr<InsetBase>(new MathSymbolInset(*this));
}


string MathSymbolInset::name() const
{
	return sym_->name;
}


void MathSymbolInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//lyxerr << "metrics: symbol: '" << sym_->name
	//	<< "' in font: '" << sym_->inset
	//	<< "' drawn as: '" << sym_->draw
	//	<< "'" << std::endl;

	int const em = mathed_char_width(mi.base.font, 'M');
	FontSetChanger dummy(mi.base, sym_->inset.c_str());
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


void MathSymbolInset::draw(PainterInfo & pi, int x, int y) const
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
	os << "[symbol " << name() << ']';
}


void MathSymbolInset::maple(MapleStream & os) const
{
	if (name() == "cdot")
		os << '*';
	else if (name() == "infty")
		os << "infinity";
	else
		os << name();
}

void MathSymbolInset::maxima(MaximaStream & os) const
{
	if (name() == "cdot")
		os << '*';
	else if (name() == "infty")
		os << "INF";
	else
		os << name();
}


void MathSymbolInset::mathematica(MathematicaStream & os) const
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


void MathSymbolInset::octave(OctaveStream & os) const
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


void MathSymbolInset::infoize2(std::ostream & os) const
{
	os << "Symbol: " << name();
}
