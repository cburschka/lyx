/**
 * \file math_tabularinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_tabularinset.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "metricsinfo.h"
#include "math_streamstr.h"
#include "Lsstream.h"

#include <iterator>

using std::vector;
using std::istringstream;
using std::getline;
using std::istream_iterator;
using std::auto_ptr;


MathTabularInset::MathTabularInset(string const & name, int m, int n)
	: MathGridInset(m, n), name_(name)
{}


MathTabularInset::MathTabularInset(string const & name, int m, int n,
		char valign, string const & halign)
	: MathGridInset(m, n, valign, halign), name_(name)
{}


MathTabularInset::MathTabularInset(string const & name, char valign,
		string const & halign)
	: MathGridInset(valign, halign), name_(name)
{}


auto_ptr<InsetBase> MathTabularInset::clone() const
{
	return auto_ptr<InsetBase>(new MathTabularInset(*this));
}


void MathTabularInset::metrics(MetricsInfo & mi, Dimension & /*dim*/) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	return MathGridInset::metrics(mi);
}


void MathTabularInset::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, "textnormal");
	MathGridInset::draw(pi, x, y);
}


void MathTabularInset::write(WriteStream & os) const
{
	if (os.fragile())
		os << "\\protect";
	os << "\\begin{" << name_ << '}';

	if (v_align_ == 't' || v_align_ == 'b')
		os << '[' << char(v_align_) << ']';
	os << '{' << halign() << "}\n";

	MathGridInset::write(os);

	if (os.fragile())
		os << "\\protect";
	os << "\\end{" << name_ << '}';
	// adding a \n here is bad if the tabular is the last item
	// in an \eqnarray...
}


void MathTabularInset::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ';
	MathGridInset::normalize(os);
	os << ']';
}


void MathTabularInset::maple(MapleStream & os) const
{
	os << "array(";
	MathGridInset::maple(os);
	os << ')';
}
