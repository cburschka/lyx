/**
 * \file math_scriptinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_scriptinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_support.h"
#include "math_symbolinset.h"
#include "dispatchresult.h"
#include "cursor.h"
#include "debug.h"
#include "funcrequest.h"

#include <boost/assert.hpp>

using std::string;
using std::max;
using std::auto_ptr;
using std::endl;



MathScriptInset::MathScriptInset()
	: MathNestInset(1), cell_1_is_up_(false), limits_(0)
{}


MathScriptInset::MathScriptInset(bool up)
	: MathNestInset(2), cell_1_is_up_(up), limits_(0)
{}


MathScriptInset::MathScriptInset(MathAtom const & at, bool up)
	: MathNestInset(2), cell_1_is_up_(up), limits_(0)
{
	BOOST_ASSERT(nargs() >= 1);
	cell(0).push_back(at);
}



auto_ptr<InsetBase> MathScriptInset::clone() const
{
	return auto_ptr<InsetBase>(new MathScriptInset(*this));
}


MathScriptInset const * MathScriptInset::asScriptInset() const
{
	return this;
}


MathScriptInset * MathScriptInset::asScriptInset()
{
	return this;
}


bool MathScriptInset::idxFirst(LCursor & cur) const
{
	cur.idx() = 0;
	cur.pos() = 0;
	return true;
}


bool MathScriptInset::idxLast(LCursor & cur) const
{
	cur.idx() = 0;
	cur.pos() = nuc().size();
	return true;
}


MathArray const & MathScriptInset::down() const
{
#if 1
	if (nargs() == 3)
		return cell(2);
	BOOST_ASSERT(nargs() > 1);
	return cell(1);
#else
	return nargs() == 3 ? cell(2) : cell(1);
#endif
}


MathArray & MathScriptInset::down()
{
#if 1
	if (nargs() == 3)
		return cell(2);
	BOOST_ASSERT(nargs() > 1);
	return cell(1);
#else
	return nargs() == 3 ? cell(2) : cell(1);
#endif
}


MathArray const & MathScriptInset::up() const
{
	BOOST_ASSERT(nargs() > 1);
	return cell(1);
}


MathArray & MathScriptInset::up()
{
	BOOST_ASSERT(nargs() > 1);
	return cell(1);
}


void MathScriptInset::ensure(bool up)
{
	if (nargs() == 1) {
		// just nucleus so far
		cells_.push_back(MathArray());
		cell_1_is_up_ = up;
	} else if (nargs() == 2 && !has(up)) {
		if (up) {
			cells_.push_back(cell(1));
			cell(1).clear();
		} else {
			cells_.push_back(MathArray());
		}
	}
}


MathArray const & MathScriptInset::nuc() const
{
	return cell(0);
}


MathArray & MathScriptInset::nuc()
{
	return cell(0);
}


int MathScriptInset::dy0() const
{
	int nd = ndes();
	if (!hasDown())
		return nd;
	int des = down().ascent();
	if (hasLimits())
		des += nd + 2;
	else
		des = max(des, nd);
	return des;
}


int MathScriptInset::dy1() const
{
	int na = nasc();
	if (!hasUp())
		return na;
	int asc = up().descent();
	if (hasLimits())
		asc += na + 2;
	else
		asc = max(asc, na);
	asc = max(asc, 5);
	return asc;
}


int MathScriptInset::dx0() const
{
	BOOST_ASSERT(hasDown());
	return hasLimits() ? (dim_.wid - down().width()) / 2 : nwid();
}


int MathScriptInset::dx1() const
{
	BOOST_ASSERT(hasUp());
	return hasLimits() ? (dim_.wid - up().width()) / 2 : nwid();
}


int MathScriptInset::dxx() const
{
	return hasLimits() ? (dim_.wid - nwid()) / 2  :  0;
}


int MathScriptInset::nwid() const
{
	return nuc().size() ? nuc().width() : 2;
}


int MathScriptInset::nasc() const
{
	return nuc().size() ? nuc().ascent() : 5;
}


int MathScriptInset::ndes() const
{
	return nuc().size() ? nuc().descent() : 0;
}


void MathScriptInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi);
	ScriptChanger dummy(mi.base);
	if (nargs() > 1)
		cell(1).metrics(mi);
	if (nargs() > 2)
		cell(2).metrics(mi);
	dim.wid = 0;
	if (hasLimits()) {
		dim.wid = nwid();
		if (hasUp())
			dim.wid = max(dim.wid, up().width());
		if (hasDown())
			dim.wid = max(dim.wid, down().width());
	} else {
		if (hasUp())
			dim.wid = max(dim.wid, up().width());
		if (hasDown())
			dim.wid = max(dim.wid, down().width());
		dim.wid += nwid();
	}
	dim.asc = dy1() + (hasUp() ? up().ascent() : 0);
	dim.des = dy0() + (hasDown() ? down().descent() : 0);
	metricsMarkers(dim);
	dim_ = dim;
}


void MathScriptInset::draw(PainterInfo & pi, int x, int y) const
{
	if (nuc().size())
		nuc().draw(pi, x + dxx(), y);
	else {
		nuc().setXY(x + dxx(), y);
		if (editing(pi.base.bv))
			drawStr(pi, pi.base.font, x + dxx(), y, ".");
	}
	ScriptChanger dummy(pi.base);
	if (hasUp())
		up().draw(pi, x + dx1(), y - dy1());
	if (hasDown())
		down().draw(pi, x + dx0(), y + dy0());
	drawMarkers(pi, x, y);
}


void MathScriptInset::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	if (hasUp())
		up().metricsT(mi, dim);
	if (hasDown())
		down().metricsT(mi, dim);
	nuc().metricsT(mi, dim);
}


void MathScriptInset::drawT(TextPainter & pain, int x, int y) const
{
	if (nuc().size())
		nuc().drawT(pain, x + dxx(), y);
	if (hasUp())
		up().drawT(pain, x + dx1(), y - dy1());
	if (hasDown())
		down().drawT(pain, x + dx0(), y + dy0());
}



bool MathScriptInset::hasLimits() const
{
	// obvious cases
	if (limits_ == 1)
		return true;
	if (limits_ == -1)
		return false;

	// we can only display limits if the nucleus wants some
	if (!nuc().size())
		return false;
	if (!nuc().back()->isScriptable())
		return false;

	// per default \int has limits beside the \int even in displayed formulas
	if (nuc().back()->asSymbolInset())
		if (nuc().back()->asSymbolInset()->name().find("int") != string::npos)
			return false;

	// assume "real" limits for everything else
	return true;
}


void MathScriptInset::removeScript(bool up)
{
	if (nargs() == 2) {
		if (up == cell_1_is_up_)
			cells_.pop_back();
	} else if (nargs() == 3) {
		if (up == true) {
			swap(cells_[1], cells_[2]);
			cell_1_is_up_ = false;
		} else {
			cell_1_is_up_ = true;
		}
	}
}


bool MathScriptInset::has(bool up) const
{
	return idxOfScript(up);
}


bool MathScriptInset::hasUp() const
{
	//lyxerr << "hasUp: " << bool(idxOfScript(true)) << endl;
	//lyxerr << "1up: " << bool(cell_1_is_up_) << endl;
	return idxOfScript(true);
}


bool MathScriptInset::hasDown() const
{
	//lyxerr << "hasDown: " << bool(idxOfScript(false)) << endl;
	//lyxerr << "1up: " << bool(cell_1_is_up_) << endl;
	return idxOfScript(false);
}


InsetBase::idx_type MathScriptInset::idxOfScript(bool up) const
{
	if (nargs() == 1)
		return 0;
	if (nargs() == 2)
		return cell_1_is_up_ == up ? 1 : 0;
	if (nargs() == 3)
		return up ? 1 : 2;
	BOOST_ASSERT(false);
	// Silence compiler
	return 0;
}


bool MathScriptInset::idxRight(LCursor &) const
{
	return false;
}


bool MathScriptInset::idxLeft(LCursor &) const
{
	return false;
}


bool MathScriptInset::idxUpDown(LCursor & cur, bool up) const
{
	// in nucleus?
	if (cur.idx() == 0) {
		// don't go up/down if there is no cell in this direction
		if (!has(up))
			return false;
		// go up/down only if in the last position
		// or in the first position of something with displayed limits
		if (cur.pos() == cur.lastpos() || (cur.pos() == 0 && hasLimits())) {
			cur.idx() = idxOfScript(up);
			cur.pos() = 0;
			return true;
		}
		return false;
	}

	// Are we 'up'?
	if (cur.idx() == idxOfScript(true)) {
		// can't go further up
		if (up)
			return false;
		// otherwise go to last position in the nucleus
		cur.idx() = 0;
		cur.pos() = cur.lastpos();
		return true;
	}

	// Are we 'down'?
	if (cur.idx() == idxOfScript(false)) {
		// can't go further down
		if (!up)
			return false;
		// otherwise go to last position in the nucleus
		cur.idx() = 0;
		cur.pos() = cur.lastpos();
		return true;
	}

	return false;
}


void MathScriptInset::write(WriteStream & os) const
{
	if (nuc().size()) {
		os << nuc();
		//if (nuc().back()->takesLimits()) {
			if (limits_ == -1)
				os << "\\nolimits ";
			if (limits_ == 1)
				os << "\\limits ";
		//}
	} else {
		if (os.firstitem())
			lyxerr[Debug::MATHED] << "suppressing {} when writing"
					      << endl;
		else
			os << "{}";
	}

	if (hasDown() && down().size())
		os << "_{" << down() << '}';

	if (hasUp() && up().size())
		os << "^{" << up() << '}';

	if (lock_ && !os.latex())
		os << "\\lyxlock ";
}


void MathScriptInset::normalize(NormalStream & os) const
{
	bool d = hasDown() && down().size();
	bool u = hasUp() && up().size();

	if (u && d)
		os << "[subsup ";
	else if (u)
		os << "[sup ";
	else if (d)
		os << "[sub ";

	if (nuc().size())
		os << nuc() << ' ';
	else
		os << "[par]";

	if (u && d)
		os << down() << ' ' << up() << ']';
	else if (d)
		os << down() << ']';
	else if (u)
		os << up() << ']';
}


void MathScriptInset::maple(MapleStream & os) const
{
	if (nuc().size())
		os << nuc();
	if (hasDown() && down().size())
		os << '[' << down() << ']';
	if (hasUp() && up().size())
		os << "^(" << up() << ')';
}


void MathScriptInset::mathematica(MathematicaStream & os) const
{
	bool d = hasDown() && down().size();
	bool u = hasUp() && up().size();

	if (nuc().size()) {
		if (d)
			os << "Subscript[" << nuc();
		else
			os << nuc();
	}

	if (u)
		os << "^(" << up() << ')';

	if (nuc().size()) {
		if (d)
			os << ',' << down() << ']';
	}
}


void MathScriptInset::mathmlize(MathMLStream & os) const
{
	bool d = hasDown() && down().size();
	bool u = hasUp() && up().size();

	if (u && d)
		os << MTag("msubsup");
	else if (u)
		os << MTag("msup");
	else if (d)
		os << MTag("msub");

	if (nuc().size())
		os << nuc();
	else
		os << "<mrow/>";

	if (u && d)
		os << down() << up() << ETag("msubsup");
	else if (u)
		os << up() << ETag("msup");
	else if (d)
		os << down() << ETag("msub");
}


void MathScriptInset::octave(OctaveStream & os) const
{
	if (nuc().size())
		os << nuc();
	if (hasDown() && down().size())
		os << '[' << down() << ']';
	if (hasUp() && up().size())
		os << "^(" << up() << ')';
}


void MathScriptInset::infoize(std::ostream & os) const
{
	os << "Scripts";
}


void MathScriptInset::infoize2(std::ostream & os) const
{
	if (limits_)
		os << (limits_ == 1 ? ", Displayed limits" : ", Inlined limits");
}


void MathScriptInset::notifyCursorLeaves(idx_type idx)
{
	MathNestInset::notifyCursorLeaves(idx);

	// remove empty scripts if possible
	if (idx == 2 && nargs() > 2 && cell(2).empty()) {
		removeScript(false); // must be a subscript...
	} else if (idx == 1 && nargs() > 1 && cell(1).empty()) {
		if (nargs() == 2) {
			cell_1_is_up_ = false;
			cell(1) = cell(2);
			cells_.pop_back();
		} else if (nargs() == 1) {
			cells_.pop_back();
		}
	}
}


void MathScriptInset::priv_dispatch(LCursor & cur, FuncRequest & cmd)
{
	//lyxerr << "MathScriptInset: request: " << cmd << std::endl;

	if (cmd.action == LFUN_MATH_LIMITS) {
		if (!cmd.argument.empty()) {
			if (cmd.argument == "limits")
				limits_ = 1;
			else if (cmd.argument == "nolimits")
				limits_ = -1;
			else
				limits_ = 0;
		} else if (limits_ == 0)
			limits_ = hasLimits() ? -1 : 1;
		else
			limits_ = 0;
		return;
	}

	MathNestInset::priv_dispatch(cur, cmd);
}
