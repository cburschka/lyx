
#include "math_scriptinset.h"
#include "math_support.h"
#include "math_symbolinset.h"
#include "math_mathmlstream.h"
#include "funcrequest.h"
#include "support/LAssert.h"
#include "debug.h"


using std::max;


MathScriptInset::MathScriptInset()
	: MathNestInset(3), limits_(0)
{
	script_[0] = false;
	script_[1] = false;
}


MathScriptInset::MathScriptInset(bool up)
	: MathNestInset(3), limits_(0)
{
	script_[0] = !up;
	script_[1] = up;
}


MathScriptInset::MathScriptInset(MathAtom const & at, bool up)
	: MathNestInset(3), limits_(0)
{
	script_[0] = !up;
	script_[1] = up;
	cell(2).push_back(at);
}



MathInset * MathScriptInset::clone() const
{
	return new MathScriptInset(*this);
}


MathScriptInset const * MathScriptInset::asScriptInset() const
{
	return this;
}


MathScriptInset * MathScriptInset::asScriptInset()
{
	return this;
}


bool MathScriptInset::idxFirst(idx_type & idx, pos_type & pos) const
{
	idx = 2;
	pos = 0;
	return true;
}


bool MathScriptInset::idxLast(idx_type & idx, pos_type & pos) const
{
	idx = 2;
	pos = nuc().size();
	return true;
}


MathArray const & MathScriptInset::down() const
{
	return cell(0);
}


MathArray & MathScriptInset::down()
{
	return cell(0);
}


MathArray const & MathScriptInset::up() const
{
	return cell(1);
}


MathArray & MathScriptInset::up()
{
	return cell(1);
}


void MathScriptInset::ensure(bool up)
{
	script_[up] = true;
}


MathArray const & MathScriptInset::nuc() const
{
	return cell(2);
}


MathArray & MathScriptInset::nuc()
{
	return cell(2);
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
	lyx::Assert(hasDown());
	return hasLimits() ? (dim_.wid - down().width()) / 2 : nwid();
}


int MathScriptInset::dx1() const
{
	lyx::Assert(hasUp());
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


Dimension MathScriptInset::metrics(MetricsInfo & mi) const
{
	cell(2).metrics(mi);
	ScriptChanger dummy(mi.base);
	cell(0).metrics(mi);
	cell(1).metrics(mi);
	dim_.wid = 0;
	if (hasLimits()) {
		dim_.wid = nwid();
		if (hasUp())
			dim_.wid = max(dim_.wid, up().width());
		if (hasDown())
			dim_.wid = max(dim_.wid, down().width());
	} else {
		if (hasUp())
			dim_.wid = max(dim_.wid, up().width());
		if (hasDown())
			dim_.wid = max(dim_.wid, down().width());
		dim_.wid += nwid();
	}
	dim_.asc = dy1() + (hasUp() ? up().ascent() : 0);
	dim_.des = dy0() + (hasDown() ? down().descent() : 0);
	metricsMarkers();
	return dim_;
}


void MathScriptInset::draw(PainterInfo & pi, int x, int y) const
{
	if (nuc().size())
		nuc().draw(pi, x + dxx(), y);
	else {
		nuc().setXY(x + dxx(), y);
		if (editing())
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
	cell(up).clear();
	script_[up] = false;
}


bool MathScriptInset::has(bool up) const
{
	return script_[up];
}


bool MathScriptInset::hasUp() const
{
	return script_[1];
}


bool MathScriptInset::hasDown() const
{
	return script_[0];
}


bool MathScriptInset::idxRight(idx_type &, pos_type &) const
{
	return false;
}


bool MathScriptInset::idxLeft(idx_type &, pos_type &) const
{
	return false;
}


bool MathScriptInset::idxUpDown(idx_type & idx, pos_type & pos, bool up,
	int) const
{
	if (idx == 1) {
		// if we are 'up' we can't go further up
		if (up)
			return false;
		// otherwise go to last base position
		idx = 2;
		pos = cell(2).size();
	}

	else if (idx == 0) {
		// if we are 'down' we can't go further down
		if (!up)
			return false;
		idx = 2;
		pos = cell(2).size();
	}

	else {
		// in nucleus
		// don't go up/down if there is no cell.
		if (!has(up))
			return false;
		// go up/down only if in the last position
		// or in the first position of something with displayed limits
		if (pos == cell(2).size() || (pos == 0 && hasLimits())) {
			idx = up;
			pos = 0;
			return true;
		}
		return false;
	}
	return true;
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
			lyxerr[Debug::MATHED] << "suppressing {} when writing\n";
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

	if (nuc().size())
		if (d)
			os << ',' << down() << ']';
}


void MathScriptInset::mathmlize( MathMLStream & os) const
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
	if (idx != 2 && script_[idx] && cell(idx).empty()) {
		cell(idx).clear();
		script_[idx] = false;
	}
}


dispatch_result MathScriptInset::dispatch
	(FuncRequest const & cmd, idx_type & idx, pos_type & pos)
{
	if (cmd.action == LFUN_MATH_LIMITS) {
		if (!cmd.argument.empty()) {
			if (cmd.argument == "limits")
				limits_ = 1;
			else if (cmd.argument == "nolimits")
				limits_ = -1;
			else
				limits_ = 0;
		} else if (limits_ == 0)
			limits_ =  (hasLimits()) ? -1 : 1;
		else
			limits_ = 0;
		return DISPATCHED;
	}

	return MathNestInset::dispatch(cmd, idx, pos);
}
