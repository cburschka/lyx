/**
 * \file InsetMathTabular.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathTabular.h"

#include "MathData.h"
#include "MathStream.h"
#include "MathStream.h"

#include "support/lstrings.h"

#include <ostream>

namespace lyx {

InsetMathTabular::InsetMathTabular(Buffer * buf, docstring const & name, int m,
		int n)
	: InsetMathGrid(buf, m, n), name_(name)
{}


InsetMathTabular::InsetMathTabular(Buffer * buf, docstring const & name, int m,
		int n, char valign, docstring const & halign)
	: InsetMathGrid(buf, m, n, valign, halign), name_(name)
{}



Inset * InsetMathTabular::clone() const
{
	return new InsetMathTabular(*this);
}


void InsetMathTabular::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Changer dummy = mi.base.changeFontSet("textnormal");
	InsetMathGrid::metrics(mi, dim);
	dim.wid += 6;
}


Dimension const InsetMathTabular::dimension(BufferView const & bv) const
{
	Dimension dim = InsetMathGrid::dimension(bv);
	dim.wid += 6;
	return dim;
}


void InsetMathTabular::draw(PainterInfo & pi, int x, int y) const
{
	Changer dummy = pi.base.changeFontSet("textnormal");
	InsetMathGrid::drawWithMargin(pi, x, y, 4, 2);
}


void InsetMathTabular::write(WriteStream & os) const
{
	ModeSpecifier specifier(os, TEXT_MODE);

	if (os.fragile())
		os << "\\protect";
	os << "\\begin{" << name_ << '}';
	bool open = os.startOuterRow();

	char const v = verticalAlignment();
	if (v == 't' || v == 'b')
		os << '[' << v << ']';
	os << '{' << horizontalAlignments() << "}\n";

	InsetMathGrid::write(os);

	if (os.fragile())
		os << "\\protect";
	os << "\\end{" << name_ << '}';
	if (open)
		os.startOuterRow();
	// adding a \n here is bad if the tabular is the last item
	// in an \eqnarray...
}


void InsetMathTabular::infoize(odocstream & os) const
{
	docstring name = name_;
	name[0] = support::uppercase(name[0]);
	os << name << ' ';
}


void InsetMathTabular::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ';
	InsetMathGrid::normalize(os);
	os << ']';
}


void InsetMathTabular::maple(MapleStream & os) const
{
	os << "array(";
	InsetMathGrid::maple(os);
	os << ')';
}


} // namespace lyx
