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
#include "math_mathmlstream.h"

#include "buffer.h"
#include "cursor.h"
#include "debug.h"
#include "BufferView.h"
#include "LaTeXFeatures.h"

using std::string;
using std::max;
using std::auto_ptr;
using std::endl;


MathMacro::MathMacro(string const & name, int numargs)
	: MathNestInset(numargs), name_(name)
{}


auto_ptr<InsetBase> MathMacro::clone() const
{
	return auto_ptr<InsetBase>(new MathMacro(*this));
}


string MathMacro::name() const
{
	return name_;
}


void MathMacro::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (!MacroTable::globalMacros().has(name())) {
		mathed_string_dim(mi.base.font, "Unknown: " + name(), dim);
	} else if (editing(mi.base.bv)) {
		asArray(MacroTable::globalMacros().get(name()).def(), tmpl_);
		LyXFont font = mi.base.font;
		augmentFont(font, "lyxtex");
		tmpl_.metrics(mi, dim);
		dim.wid += mathed_string_width(font, name()) + 10;
		int ww = mathed_string_width(font, "#1: ");
		for (idx_type i = 0; i < nargs(); ++i) {
			MathArray const & c = cell(i);
			c.metrics(mi);
			dim.wid  = max(dim.wid, c.width() + ww);
			dim.des += c.height() + 10;
		}
	} else {
		MacroTable::globalMacros().get(name()).expand(cells_, expanded_);
		expanded_.metrics(mi, dim);
	}
	metricsMarkers2(dim);
	dim_ = dim;
}


void MathMacro::draw(PainterInfo & pi, int x, int y) const
{
	if (!MacroTable::globalMacros().has(name())) {
		drawStrRed(pi, x, y, "Unknown: " + name());
	} else if (editing(pi.base.bv)) {
		LyXFont font = pi.base.font;
		augmentFont(font, "lyxtex");
		int h = y - dim_.ascent() + 2 + tmpl_.ascent();
		drawStr(pi, font, x + 3, h, name());
		int const w = mathed_string_width(font, name());
		tmpl_.draw(pi, x + w + 12, h);
		h += tmpl_.descent();
		Dimension ldim;
		mathed_string_dim(font, "#1: ", ldim);
		for (idx_type i = 0; i < nargs(); ++i) {
			MathArray const & c = cell(i);
			h += max(c.ascent(), ldim.asc) + 5;
			c.draw(pi, x + ldim.wid, h);
			char str[] = "#1:";
			str[1] += static_cast<char>(i);
			drawStr(pi, font, x + 3, h, str);
			h += max(c.descent(), ldim.des) + 5;
		}
	} else {
		expanded_.draw(pi, x, y);
	}
	drawMarkers2(pi, x, y);
}


void MathMacro::validate(LaTeXFeatures & features) const
{
	if (name() == "binom" || name() == "mathcircumflex")
		features.require(name());
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
	//expanded_.substitute(*this);
}


void MathMacro::infoize(std::ostream & os) const
{
	os << "Macro: " << name();
}


void MathMacro::infoize2(std::ostream & os) const
{
	os << "Macro: " << name();

}
