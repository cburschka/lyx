/**
 * \file math_macro.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_macro.h"
#include "math_support.h"
#include "math_extern.h"
#include "math_macrotable.h"
#include "math_macrotemplate.h"
#include "math_mathmlstream.h"
#include "debug.h"
#include "LaTeXFeatures.h"


using std::max;
using std::auto_ptr;
using std::endl;


MathMacro::MathMacro(string const & name)
	: MathNestInset(MathMacroTable::provide(name)->asMacroTemplate()->numargs()),
		tmplate_(MathMacroTable::provide(name))
{}


MathMacro::MathMacro(MathMacro const & m)
	: MathNestInset(m),
		tmplate_(m.tmplate_) // don't copy 'expanded_'!
{}



auto_ptr<InsetBase> MathMacro::clone() const
{
	return auto_ptr<InsetBase>(new MathMacro(*this));
}


string MathMacro::name() const
{
	return tmplate_->asMacroTemplate()->name();
}


bool MathMacro::defining() const
{
	return 0;
	//return mathcursor && mathcursor->formula()->getInsetName() == name();
}


void MathMacro::expand() const
{
	expanded_ = tmplate_->cell(tmplate_->cell(1).empty() ? 0 : 1);
}


void MathMacro::metrics(MetricsInfo & mi, Dimension & dim) const
{
	augmentFont(font_, "lyxtex");
	mi_ = mi;

	if (defining()) {

		mathed_string_dim(font_, name(), dim_);

	} else if (editing()) {

		expand();
		expanded_.metrics(mi_, dim_);
		metricsMarkers();

		dim_.wid +=  mathed_string_width(font_, name()) + 10;

		int ww = mathed_string_width(font_, "#1: ");

		for (idx_type i = 0; i < nargs(); ++i) {
			MathArray const & c = cell(i);
			c.metrics(mi_);
			dim_.wid  = max(dim_.wid, c.width() + ww);
			dim_.des += max(c.ascent(),  dim_.asc) + 5;
			dim_.des += max(c.descent(), dim_.des) + 5;
		}

	} else {

		expand();
		expanded_.substitute(*this);
		expanded_.metrics(mi_, dim_);

	}

	dim = dim_;
}


void MathMacro::draw(PainterInfo & pi, int x, int y) const
{
	metrics(mi_, dim_);

	LyXFont texfont;
	augmentFont(texfont, "lyxtex");

	if (defining()) {
		drawStr(pi, texfont, x, y, name());
		return;
	}

	if (editing()) {
		int h = y - dim_.ascent() + 2 + expanded_.ascent();
		drawStr(pi, font_, x + 3, h, name());

		int const w = mathed_string_width(font_, name());
		expanded_.draw(pi, x + w + 12, h);
		h += expanded_.descent();

		Dimension ldim;
		mathed_string_dim(font_, "#1: ", ldim);

		for (idx_type i = 0; i < nargs(); ++i) {
			MathArray const & c = cell(i);
			h += max(c.ascent(), ldim.asc) + 5;
			c.draw(pi, x + ldim.wid, h);
			char str[] = "#1:";
			str[1] += static_cast<char>(i);
			drawStr(pi, texfont, x + 3, h, str);
			h += max(c.descent(), ldim.des) + 5;
		}
		return;
	}

	expanded_.draw(pi, x, y);
}


void MathMacro::dump() const
{
	MathMacroTable::dump();
	lyxerr << "\n macro: '" << this << "'\n"
	       << " name: '" << name() << "'\n"
	       << " template: '";
	WriteStream wi(lyxerr);
	tmplate_->write(wi);
	lyxerr << "'" << endl;
}


bool MathMacro::idxUpDown(idx_type & idx, pos_type &, bool up, int x) const
{
	pos_type pos;
	if (up) {
		if (!MathNestInset::idxLeft(idx, pos))
			return false;
		pos = cell(idx).x2pos(x);
		return true;
	} else {
		if (!MathNestInset::idxRight(idx, pos))
			return false;
		pos = cell(idx).x2pos(x);
		return true;
	}
}


bool MathMacro::idxLeft(idx_type &, pos_type &) const
{
	return false;
}


bool MathMacro::idxRight(idx_type &, pos_type &) const
{
	return false;
}


void MathMacro::validate(LaTeXFeatures & features) const
{
	if (name() == "binom" || name() == "mathcircumflex")
		features.require(name());
	//MathInset::validate(features);
}


void MathMacro::maple(MapleStream & os) const
{
	updateExpansion();
	::maple(expanded_, os);
}


void MathMacro::mathmlize(MathMLStream & os) const
{
	updateExpansion();
	::mathmlize(expanded_, os);
}


void MathMacro::octave(OctaveStream & os) const
{
	updateExpansion();
	::octave(expanded_, os);
}


void MathMacro::updateExpansion() const
{
	expand();
	expanded_.substitute(*this);
}


void MathMacro::infoize(std::ostream & os) const
{
	os << "Macro: " << name();
}


void MathMacro::infoize2(std::ostream & os) const
{
	os << "Macro: " << name();
}
