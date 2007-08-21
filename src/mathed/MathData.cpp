/**
 * \file MathData.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MathData.h"
#include "InsetMathFont.h"
#include "InsetMathScript.h"
#include "MathMacro.h"
#include "MacroTable.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "ReplaceData.h"

#include "BufferView.h"
#include "Buffer.h"
#include "Cursor.h"
#include "debug.h"
#include "Color.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include <boost/assert.hpp>


namespace lyx {

using std::abs;
using std::endl;
using std::min;
using std::ostringstream;
using std::string;
using std::vector;


MathData::MathData(const_iterator from, const_iterator to)
	: base_type(from, to)
{}


MathAtom & MathData::operator[](pos_type pos)
{
	BOOST_ASSERT(pos < size());
	return base_type::operator[](pos);
}


MathAtom const & MathData::operator[](pos_type pos) const
{
	BOOST_ASSERT(pos < size());
	return base_type::operator[](pos);
}


void MathData::insert(size_type pos, MathAtom const & t)
{
	base_type::insert(begin() + pos, t);
}


void MathData::insert(size_type pos, MathData const & ar)
{
	BOOST_ASSERT(pos <= size());
	base_type::insert(begin() + pos, ar.begin(), ar.end());
}


void MathData::append(MathData const & ar)
{
	insert(size(), ar);
}


void MathData::erase(size_type pos)
{
	if (pos < size())
		erase(pos, pos + 1);
}


void MathData::erase(iterator pos1, iterator pos2)
{
	base_type::erase(pos1, pos2);
}


void MathData::erase(iterator pos)
{
	base_type::erase(pos);
}


void MathData::erase(size_type pos1, size_type pos2)
{
	base_type::erase(begin() + pos1, begin() + pos2);
}


void MathData::dump2() const
{
	odocstringstream os;
	NormalStream ns(os);
	for (const_iterator it = begin(); it != end(); ++it)
		ns << *it << ' ';
	lyxerr << to_utf8(os.str());
}


void MathData::dump() const
{
	odocstringstream os;
	NormalStream ns(os);
	for (const_iterator it = begin(); it != end(); ++it)
		ns << '<' << *it << '>';
	lyxerr << to_utf8(os.str());
}


void MathData::validate(LaTeXFeatures & features) const
{
	for (const_iterator it = begin(); it != end(); ++it)
		(*it)->validate(features);
}


bool MathData::match(MathData const & ar) const
{
	return size() == ar.size() && matchpart(ar, 0);
}


bool MathData::matchpart(MathData const & ar, pos_type pos) const
{
	if (size() < ar.size() + pos)
		return false;
	const_iterator it = begin() + pos;
	for (const_iterator jt = ar.begin(); jt != ar.end(); ++jt, ++it)
		if (asString(*it) != asString(*jt))
			return false;
	return true;
}


void MathData::replace(ReplaceData & rep)
{
	for (size_type i = 0; i < size(); ++i) {
		if (find1(rep.from, i)) {
			// match found
			lyxerr << "match found!" << endl;
			erase(i, i + rep.from.size());
			insert(i, rep.to);
		}
	}

	// FIXME: temporarily disabled
	// for (const_iterator it = begin(); it != end(); ++it)
	//	it->nucleus()->replace(rep);
}


bool MathData::find1(MathData const & ar, size_type pos) const
{
	lyxerr << "finding '" << ar << "' in '" << *this << "'" << endl;
	for (size_type i = 0, n = ar.size(); i < n; ++i)
		if (asString(operator[](pos + i)) != asString(ar[i]))
			return false;
	return true;
}


MathData::size_type MathData::find(MathData const & ar) const
{
	for (int i = 0, last = size() - ar.size(); i < last; ++i)
		if (find1(ar, i))
			return i;
	return size();
}


MathData::size_type MathData::find_last(MathData const & ar) const
{
	for (int i = size() - ar.size(); i >= 0; --i)
		if (find1(ar, i))
			return i;
	return size();
}


bool MathData::contains(MathData const & ar) const
{
	if (find(ar) != size())
		return true;
	for (const_iterator it = begin(); it != end(); ++it)
		if ((*it)->contains(ar))
			return true;
	return false;
}


void MathData::touch() const
{
}


bool MathData::metrics(MetricsInfo & mi, Dimension & dim) const
{
	dim = dim_;
	metrics(mi);
	if (dim_ == dim)
		return false;
	dim = dim_;
	return true;
}


namespace {

bool isInside(DocIterator const & it, MathData const & ar,
	pos_type p1, pos_type p2)
{
	for (size_t i = 0; i != it.depth(); ++i) {
		CursorSlice const & sl = it[i];
		if (sl.inset().inMathed() && &sl.cell() == &ar)
			return p1 <= sl.pos() && sl.pos() < p2;
	}
	return false;
}

}



void MathData::metrics(MetricsInfo & mi) const
{
	frontend::FontMetrics const & fm = theFontMetrics(mi.base.font);
	dim_ = fm.dimension('I');
	int xascent = fm.dimension('x').ascent();
	if (xascent >= dim_.asc)
		xascent = (2 * dim_.asc) / 3;
	minasc_ = xascent;
	mindes_ = (3 * xascent) / 4;
	slevel_ = (4 * xascent) / 5;
	sshift_ = xascent / 4;
	kerning_ = 0;

	if (empty())
		return;

	dim_.asc = 0;
	dim_.wid = 0;
	Dimension d;
	//BufferView & bv  = *mi.base.bv;
	//Buffer const & buf = bv.buffer();
	for (size_t i = 0, n = size(); i != n; ++i) {
		MathAtom const & at = operator[](i);
#if 0
		MathMacro const * mac = at->asMacro();
		if (mac && buf.hasMacro(mac->name())) {
			MacroData const & tmpl = buf.getMacro(mac->name());
			int numargs = tmpl.numargs();
			if (i + numargs > n)
				numargs = n - i - 1;
			lyxerr << "metrics:found macro: " << mac->name()
				<< " numargs: " << numargs << endl;
			if (!isInside(bv.cursor(), *this, i + 1, i + numargs + 1)) {
				MathData args(begin() + i + 1, begin() + i + numargs + 1);
				MathData exp;
				tmpl.expand(args, exp);
				mac->setExpansion(exp, args);
				mac->metricsExpanded(mi, d);
				dim_.wid += mac->widthExpanded();
				i += numargs;
				continue;
			}
		}
#endif
		at->metrics(mi, d);
		dim_ += d;
		if (i == n - 1)
			kerning_ = at->kerning();
	}
}


void MathData::draw(PainterInfo & pi, int x, int y) const
{
	//lyxerr << "MathData::draw: x: " << x << " y: " << y << endl;
	BufferView & bv  = *pi.base.bv;
	setXY(bv, x, y);

	if (empty()) {
		pi.pain.rectangle(x, y - ascent(), width(), height(), Color::mathline);
		return;
	}

	// don't draw outside the workarea
	if (y + descent() <= 0
		|| y - ascent() >= bv.workHeight()
		|| x + width() <= 0
		|| x >= bv. workWidth())
		return;

	for (size_t i = 0, n = size(); i != n; ++i) {
		MathAtom const & at = operator[](i);
#if 0
	Buffer const & buf = bv.buffer();
		// special macro handling
		MathMacro const * mac = at->asMacro();
		if (mac && buf.hasMacro(mac->name())) {
			MacroData const & tmpl = buf.getMacro(mac->name());
			int numargs = tmpl.numargs();
			if (i + numargs > n)
				numargs = n - i - 1;
			if (!isInside(bv.cursor(), *this, i + 1, i + numargs + 1)) {
				mac->drawExpanded(pi, x, y);
				x += mac->widthExpanded();
				i += numargs;
				continue;
			}
		}
#endif
		bv.coordCache().insets().add(at.nucleus(), x, y);
		at->drawSelection(pi, x, y);
		at->draw(pi, x, y);
		x += at->width();
	}
}


void MathData::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	dim.clear();
	Dimension d;
	for (const_iterator it = begin(); it != end(); ++it) {
		(*it)->metricsT(mi, d);
		dim += d;
	}
}


void MathData::drawT(TextPainter & pain, int x, int y) const
{
	//lyxerr << "x: " << x << " y: " << y << ' ' << pain.workAreaHeight() << endl;

	// FIXME: Abdel 16/10/2006
	// This drawT() method is never used, this is dead code.

	for (const_iterator it = begin(), et = end(); it != et; ++it) {
		(*it)->drawT(pain, x, y);
		//x += (*it)->width_;
		x += 2;
	}
}


int MathData::pos2x(size_type pos) const
{
	return pos2x(pos, 0);
}


int MathData::pos2x(size_type pos, int glue) const
{
	int x = 0;
	size_type target = min(pos, size());
	for (size_type i = 0; i < target; ++i) {
		const_iterator it = begin() + i;
		if ((*it)->getChar() == ' ')
			x += glue;
		//lyxerr << "char: " << (*it)->getChar()
		//	<< "width: " << (*it)->width() << std::endl;
		x += (*it)->width();
	}
	return x;
}


MathData::size_type MathData::x2pos(int targetx) const
{
	return x2pos(targetx, 0);
}


MathData::size_type MathData::x2pos(int targetx, int glue) const
{
	const_iterator it = begin();
	int lastx = 0;
	int currx = 0;
	// find first position after targetx
	for (; currx < targetx && it < end(); ++it) {
		lastx = currx;
		if ((*it)->getChar() == ' ')
			currx += glue;
		currx += (*it)->width();
	}

	/**
	 * If we are not at the beginning of the array, go to the left
	 * of the inset if one of the following two condition holds:
	 * - the current inset is editable (so that the cursor tip is
	 *   deeper than us): in this case, we want all intermediate
	 *   cursor slices to be before insets;
	 * - the mouse is closer to the left side of the inset than to
	 *   the right one.
	 * See bug 1918 for details.
	 **/
	if (it != begin() && currx >= targetx
	    && ((*boost::prior(it))->asNestInset()
		|| abs(lastx - targetx) < abs(currx - targetx))) {
		--it;
	}

	return it - begin();
}


int MathData::dist(BufferView const & bv, int x, int y) const
{
	int xx = 0;
	int yy = 0;

	const int xo_ = xo(bv);
	const int yo_ = yo(bv);

	if (x < xo_)
		xx = xo_ - x;
	else if (x > xo_ + width())
		xx = x - xo_ - width();

	if (y < yo_ - ascent())
		yy = yo_ - ascent() - y;
	else if (y > yo_ + descent())
		yy = y - yo_ - descent();

	return xx + yy;
}


void MathData::setXY(BufferView & bv, int x, int y) const
{
	//lyxerr << "setting position cache for MathData " << this << std::endl;
	bv.coordCache().arrays().add(this, x, y);
}


int MathData::xo(BufferView const & bv) const
{
	return bv.coordCache().getArrays().x(this);
}


int MathData::yo(BufferView const & bv) const
{
	return bv.coordCache().getArrays().y(this);
}


std::ostream & operator<<(std::ostream & os, MathData const & ar)
{
	odocstringstream oss;
	NormalStream ns(oss);
	ns << ar;
	return os << to_utf8(oss.str());
}


odocstream & operator<<(odocstream & os, MathData const & ar)
{
	NormalStream ns(os);
	ns << ar;
	return os;
}


} // namespace lyx
