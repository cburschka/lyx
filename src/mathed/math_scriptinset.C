#ifdef __GNUG__
#pragma implementation
#endif

#include "math_scriptinset.h"
#include "math_support.h"
#include "math_symbolinset.h"
#include "math_mathmlstream.h"
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
	return hasLimits() ? (dim_.w - down().width()) / 2 : nwid();
}


int MathScriptInset::dx1() const
{
	lyx::Assert(hasUp());
	return hasLimits() ? (dim_.w - up().width()) / 2 : nwid();
}


int MathScriptInset::dxx() const
{
	return hasLimits() ? (dim_.w - nwid()) / 2  :  0;
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


void MathScriptInset::metrics(MathMetricsInfo & mi) const
{
	cell(2).metrics(mi);
	MathScriptChanger dummy(mi.base);
	cell(0).metrics(mi);
	cell(1).metrics(mi);
	dim_.w = 0;
	if (hasLimits()) {
		dim_.w = nwid();
		if (hasUp())
			dim_.w = max(dim_.w, up().width());
		if (hasDown())
			dim_.w = max(dim_.w, down().width());
	} else {
		if (hasUp())
			dim_.w = max(dim_.w, up().width());
		if (hasDown())
			dim_.w = max(dim_.w, down().width());
		dim_.w += nwid();
	}
	dim_.a = dy1() + (hasUp() ? up().ascent() : 0);
	dim_.d = dy0() + (hasDown() ? down().descent() : 0);
	metricsMarkers2();
}


void MathScriptInset::draw(MathPainterInfo & pi, int x, int y) const
{
	if (nuc().size())
		nuc().draw(pi, x + dxx(), y);
	else if (editing())
		drawStr(pi, pi.base.font, x + dxx(), y, ".");
	MathScriptChanger dummy(pi.base);
	if (hasUp())
		up().draw(pi, x + dx1(), y - dy1());
	if (hasDown())
		down().draw(pi, x + dx0(), y + dy0());
	drawMarkers2(pi, x, y);
}


void MathScriptInset::metricsT(TextMetricsInfo const & mi) const
{
	if (hasUp())
		up().metricsT(mi);
	if (hasDown())
		down().metricsT(mi);
	nuc().metricsT(mi);
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


void MathScriptInset::removeEmptyScripts()
{
	for (int i = 0; i <= 1; ++i)
		if (script_[i] && cell(i).size() == 0) {
			cell(i).clear();
			script_[i] = false;
		}
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
		// don't go up/down unless in last position
		if (pos != cell(2).size())
			return false;	
		// don't go up/down if there is no cell.
		if (!has(up))
			return false;
		// otherwise move into the first position
		idx = up;
		pos = 0;
	}
	return true;
}


void MathScriptInset::write(WriteStream & os) const
{
	if (nuc().size()) {
		os << nuc();
		if (nuc().back()->takesLimits()) {
			if (limits_ == -1)
				os << "\\nolimits ";
			if (limits_ == 1)
				os << "\\limits ";
		}
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


void MathScriptInset::maplize(MapleStream & os) const
{
	if (nuc().size())
		os << nuc();
	if (hasDown() && down().size())
		os << '[' << down() << ']';
	if (hasUp() && up().size())
		os << "^(" << up() << ')';
}


void MathScriptInset::mathematicize(MathematicaStream & os) const
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
		os << "^(" << up() << ")";

	if (nuc().size())
		if (d)
			os << "," << down() << "]"; 
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


void MathScriptInset::octavize(OctaveStream & os) const
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
	if (limits_)
		os << (limits_ == 1 ? ", Displayed limits" : ", Inlined limits");
}
