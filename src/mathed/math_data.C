/**
 * \file math_data.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_data.h"
#include "math_fontinset.h"
#include "math_scriptinset.h"
#include "math_macro.h"
#include "math_macrotable.h"
#include "math_mathmlstream.h"
#include "math_support.h"
#include "math_replace.h"

#include "coordcache.h"
#include "LColor.h"
#include "BufferView.h"
#include "buffer.h"
#include "cursor.h"
#include "debug.h"

#include "frontends/Painter.h"

#include <boost/assert.hpp>

using std::abs;
using std::endl;
using std::min;
using std::ostringstream;
using std::string;
using std::vector;


MathArray::MathArray()
{}


MathArray::MathArray(const_iterator from, const_iterator to)
	: base_type(from, to)
{}


MathAtom & MathArray::operator[](pos_type pos)
{
	BOOST_ASSERT(pos < size());
	return base_type::operator[](pos);
}


MathAtom const & MathArray::operator[](pos_type pos) const
{
	BOOST_ASSERT(pos < size());
	return base_type::operator[](pos);
}


void MathArray::insert(size_type pos, MathAtom const & t)
{
	base_type::insert(begin() + pos, t);
}


void MathArray::insert(size_type pos, MathArray const & ar)
{
	BOOST_ASSERT(pos <= size());
	base_type::insert(begin() + pos, ar.begin(), ar.end());
}


void MathArray::append(MathArray const & ar)
{
	insert(size(), ar);
}


void MathArray::erase(size_type pos)
{
	if (pos < size())
		erase(pos, pos + 1);
}


void MathArray::erase(iterator pos1, iterator pos2)
{
	base_type::erase(pos1, pos2);
}


void MathArray::erase(iterator pos)
{
	base_type::erase(pos);
}


void MathArray::erase(size_type pos1, size_type pos2)
{
	base_type::erase(begin() + pos1, begin() + pos2);
}


void MathArray::dump2() const
{
	NormalStream ns(lyxerr);
	for (const_iterator it = begin(); it != end(); ++it)
		ns << *it << ' ';
}


void MathArray::dump() const
{
	NormalStream ns(lyxerr);
	for (const_iterator it = begin(); it != end(); ++it)
		ns << '<' << *it << '>';
}


void MathArray::validate(LaTeXFeatures & features) const
{
	for (const_iterator it = begin(); it != end(); ++it)
		(*it)->validate(features);
}


bool MathArray::match(MathArray const & ar) const
{
	return size() == ar.size() && matchpart(ar, 0);
}


bool MathArray::matchpart(MathArray const & ar, pos_type pos) const
{
	if (size() < ar.size() + pos)
		return false;
	const_iterator it = begin() + pos;
	for (const_iterator jt = ar.begin(); jt != ar.end(); ++jt, ++it)
		if (asString(*it) != asString(*jt))
			return false;
	return true;
}


void MathArray::replace(ReplaceData & rep)
{
	for (size_type i = 0; i < size(); ++i) {
		if (find1(rep.from, i)) {
			// match found
			lyxerr << "match found!" << endl;
			erase(i, i + rep.from.size());
			insert(i, rep.to);
		}
	}

#ifdef WITH_WARNINGS
#warning temporarily disabled
	// for (const_iterator it = begin(); it != end(); ++it)
	//	it->nucleus()->replace(rep);
#endif
}


bool MathArray::find1(MathArray const & ar, size_type pos) const
{
	lyxerr << "finding '" << ar << "' in '" << *this << "'" << endl;
	for (size_type i = 0, n = ar.size(); i < n; ++i)
		if (asString(operator[](pos + i)) != asString(ar[i]))
			return false;
	return true;
}


MathArray::size_type MathArray::find(MathArray const & ar) const
{
	for (int i = 0, last = size() - ar.size(); i < last; ++i)
		if (find1(ar, i))
			return i;
	return size();
}


MathArray::size_type MathArray::find_last(MathArray const & ar) const
{
	for (int i = size() - ar.size(); i >= 0; --i)
		if (find1(ar, i))
			return i;
	return size();
}


bool MathArray::contains(MathArray const & ar) const
{
	if (find(ar) != size())
		return true;
	for (const_iterator it = begin(); it != end(); ++it)
		if ((*it)->contains(ar))
			return true;
	return false;
}


void MathArray::touch() const
{
}


void MathArray::metrics(MetricsInfo & mi, Dimension & dim) const
{
	metrics(mi);
	dim = dim_;
}


namespace {

bool isInside(DocIterator const & it, MathArray const & ar,
	lyx::pos_type p1, lyx::pos_type p2)
{
	for (size_t i = 0; i != it.size(); ++i) {
		CursorSlice const & sl = it[i];
		if (sl.inset().inMathed() && &sl.cell() == &ar)
			return p1 <= sl.pos() && sl.pos() < p2;
	}
	return false;
}

}



void MathArray::metrics(MetricsInfo & mi) const
{
	mathed_char_dim(mi.base.font, 'I', dim_);

	if (empty())
		return;

	dim_.wid = 0;
	Dimension d;
	//BufferView & bv  = *mi.base.bv;
	//Buffer const & buf = *bv.buffer();
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
				MathArray args(begin() + i + 1, begin() + i + numargs + 1);
				MathArray exp;
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
	}
}


void MathArray::draw(PainterInfo & pi, int x, int y) const
{
	//lyxerr << "MathArray::draw: x: " << x << " y: " << y << endl;
	setXY(x, y);

	if (empty()) {
		pi.pain.rectangle(x, y - ascent(), width(), height(), LColor::mathline);
		return;
	}

	// don't draw outside the workarea
	if (y + descent() <= 0
		|| y - ascent() >= pi.pain.paperHeight()
		|| x + width() <= 0
		|| x >= pi.pain.paperWidth())
		return;

	//BufferView & bv  = *pi.base.bv;
	for (size_t i = 0, n = size(); i != n; ++i) {
		MathAtom const & at = operator[](i);
#if 0
	Buffer const & buf = *bv.buffer();
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
		at->drawSelection(pi, x, y);
		at->draw(pi, x, y);
		x += at->width();
	}
}


void MathArray::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	dim.clear();
	Dimension d;
	for (const_iterator it = begin(); it != end(); ++it) {
		(*it)->metricsT(mi, d);
		dim += d;
	}
}


void MathArray::drawT(TextPainter & pain, int x, int y) const
{
	//lyxerr << "x: " << x << " y: " << y << ' ' << pain.workAreaHeight() << endl;
	setXY(x, y);

	for (const_iterator it = begin(), et = end(); it != et; ++it) {
		(*it)->drawT(pain, x, y);
		//x += (*it)->width_;
		x += 2;
	}
}


int MathArray::pos2x(size_type pos) const
{
	return pos2x(pos, 0);
}


int MathArray::pos2x(size_type pos, int glue) const
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


MathArray::size_type MathArray::x2pos(int targetx) const
{
	return x2pos(targetx, 0);
}


MathArray::size_type MathArray::x2pos(int targetx, int glue) const
{
	const_iterator it = begin();
	int lastx = 0;
	int currx = 0;
	for (; currx < targetx && it < end(); ++it) {
		lastx = currx;
		if ((*it)->getChar() == ' ')
			currx += glue;
		currx += (*it)->width();
	}
	if (abs(lastx - targetx) < abs(currx - targetx) && it != begin())
		--it;
	return it - begin();
}


int MathArray::dist(int x, int y) const
{
	int xx = 0;
	int yy = 0;

	const int xo_ = xo();
	const int yo_ = yo();

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


void MathArray::setXY(int x, int y) const
{
	lyxerr << "setting position cache for MathArray " << this << std::endl;
	theCoords.arrays_.add(this, x, y);
}


int MathArray::xo() const
{
	return theCoords.arrays_.x(this);
}


int MathArray::yo() const
{
	return theCoords.arrays_.y(this);
}

