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
	: MathNestInset(2), limits_(0)
{
	script_[0] = false;
	script_[1] = false;
}


MathScriptInset::MathScriptInset(bool up)
	: MathNestInset(2), limits_(0)
{
	script_[0] = !up;
	script_[1] = up;
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


MathXArray const & MathScriptInset::up() const
{
	return xcell(1);
}


MathXArray const & MathScriptInset::down() const
{
	return xcell(0);
}


MathXArray & MathScriptInset::up()
{
	return xcell(1);
}


MathXArray & MathScriptInset::down()
{
	return xcell(0);
}


void MathScriptInset::ensure(bool up)
{
	script_[up] = true;
}


int MathScriptInset::dy0(MathInset const * nuc) const
{
	int nd = ndes(nuc);
	if (!hasDown())
		return nd;
	int des = down().ascent();
	if (hasLimits(nuc))
		des += nd + 2;
	else 
		des = max(des, nd);
	return des;
}


int MathScriptInset::dy1(MathInset const * nuc) const
{
	int na = nasc(nuc);
	if (!hasUp())
		return na;
	int asc = up().descent();
	if (hasLimits(nuc))
		asc += na + 2;
	else 
		asc = max(asc, na);
	asc = max(asc, mathed_char_ascent(font_, 'I'));
	return asc;
}


int MathScriptInset::dx0(MathInset const * nuc) const
{
	lyx::Assert(hasDown());
	return hasLimits(nuc) ? (width2(nuc) - down().width()) / 2 : nwid(nuc);
}


int MathScriptInset::dx1(MathInset const * nuc) const
{
	lyx::Assert(hasUp());
	return hasLimits(nuc) ? (width2(nuc) - up().width()) / 2 : nwid(nuc);
}


int MathScriptInset::dxx(MathInset const * nuc) const
{
	//lyx::Assert(nuc());
	return hasLimits(nuc)  ?  (width2(nuc) - nwid(nuc)) / 2  :  0;
}


int MathScriptInset::ascent2(MathInset const * nuc) const
{
	return dy1(nuc) + (hasUp() ? up().ascent() : 0);
}


int MathScriptInset::descent2(MathInset const * nuc) const
{
	return dy0(nuc) + (hasDown() ? down().descent() : 0);
}


int MathScriptInset::width2(MathInset const * nuc) const
{
	int wid = 0;
	if (hasLimits(nuc)) {
		wid = nwid(nuc);
		if (hasUp())
			wid = max(wid, up().width());
		if (hasDown())
			wid = max(wid, down().width());
	} else {
		if (hasUp())
			wid = max(wid, up().width());
		if (hasDown())
			wid = max(wid, down().width());
		wid += nwid(nuc);
	}
	return wid;
}


int MathScriptInset::nwid(MathInset const * nuc) const
{
	return nuc ?  nuc->width() : mathed_char_width(font_, '.');
}


int MathScriptInset::nasc(MathInset const * nuc) const
{
	return nuc ? nuc->ascent() : mathed_char_ascent(font_, 'I');
}


int MathScriptInset::ndes(MathInset const * nuc) const
{
	return nuc ? nuc->descent() : mathed_char_descent(font_, 'I');
}


void MathScriptInset::metrics(MathMetricsInfo const & mi) const
{	
	metrics(0, mi);
}


void MathScriptInset::metrics(MathInset const * nuc,
	MathMetricsInfo const & mi) const
{	
	MathMetricsInfo m = mi;
	smallerStyleScript(m);
	MathNestInset::metrics(m);
	whichFont(font_, LM_TC_VAR, m);
	if (nuc)
		nuc->metrics(mi);
	ascent_  = ascent2(nuc);
	descent_ = descent2(nuc);
	width_   = width2(nuc);
}


void MathScriptInset::metrics(TextMetricsInfo const & mi) const
{	
	metrics(0, mi);
}


void MathScriptInset::metrics(MathInset const * nuc,
	TextMetricsInfo const & mi) const
{	
	if (hasUp())
		up().metrics(mi);
	if (hasDown())
		down().metrics(mi);
	if (nuc)
		nuc->metrics(mi);
	//ascent_  = ascent2(nuc);
	//descent_ = descent2(nuc);
	//width_   = width2(nuc);
}


void MathScriptInset::draw(Painter & pain, int x, int y) const
{  
	//lyxerr << "unexpected call to MathScriptInset::draw()\n";
	draw(0, pain, x, y);
}


void MathScriptInset::draw(MathInset const * nuc, Painter & pain,
	int x, int y) const
{  
	if (nuc)
		nuc->draw(pain, x + dxx(nuc), y);
	else // if (editing())
		drawStr(pain, font_, x + dxx(nuc), y, ".");

	if (hasUp())
		up().draw(pain, x + dx1(nuc), y - dy1(nuc));

	if (hasDown())
		down().draw(pain, x + dx0(nuc), y + dy0(nuc));
}

void MathScriptInset::draw(TextPainter & pain, int x, int y) const
{  
	//lyxerr << "unexpected call to MathScriptInset::draw()\n";
	draw(0, pain, x, y);
}


void MathScriptInset::draw(MathInset const * nuc, TextPainter & pain,
	int x, int y) const
{  
	if (nuc)
		nuc->draw(pain, x + dxx(nuc), y);
	if (hasUp())
		up().draw(pain, x + dx1(nuc), y - dy1(nuc));
	if (hasDown())
		down().draw(pain, x + dx0(nuc), y + dy0(nuc));
}



bool MathScriptInset::hasLimits(MathInset const * nuc) const
{
	// obviuos cases
	if (limits_ == 1)
		return true;
	if (limits_ == -1)
		return false;

	// we can only display limits if the nucleus wants some	
	if (!nuc)
		return false;
	if (!nuc->isScriptable())
		return false;
	
	// per default \int has limits beside the \int even in displayed formulas
	if (nuc->asSymbolInset())
		if (nuc->asSymbolInset()->name().find("int") != string::npos)
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


bool MathScriptInset::empty() const
{
	return !script_[0] && !script_[1];
}


bool MathScriptInset::hasUp() const
{
	return script_[1];
}


bool MathScriptInset::hasDown() const
{
	return script_[0];
}


bool MathScriptInset::idxRight(MathInset::idx_type &,
				 MathInset::pos_type &) const
{
	return false;
}


bool MathScriptInset::idxLeft(MathInset::idx_type &,
				MathInset::pos_type &) const
{
	return false;
}


void MathScriptInset::write(WriteStream & os) const
{  
	//lyxerr << "unexpected call to MathScriptInset::write()\n";
	write2(0, os);
}


void MathScriptInset::write2(MathInset const * nuc, WriteStream & os) const
{
	if (nuc) {
		os << nuc;
		if (nuc->takesLimits()) {
			if (limits_ == -1)
				os << "\\nolimits ";
			if (limits_ == 1)
				os << "\\limits ";
		}
	} else
			if (os.firstitem())
				lyxerr << "suppressing {} \n";
			else
				os << "{}";

	if (hasDown() && down().data_.size())
		os << "_{" << down().data_ << '}';

	if (hasUp() && up().data_.size())
		os << "^{" << up().data_ << '}';
}


void MathScriptInset::normalize(NormalStream & os) const
{  
	//lyxerr << "unexpected call to MathScriptInset::normalize()\n";
	normalize2(0, os);
}


void MathScriptInset::normalize2(MathInset const * nuc, NormalStream & os) const
{
	bool d = hasDown() && down().data_.size();
	bool u = hasUp() && up().data_.size();

	if (u) 
		os << "[sup ";
	if (d)
		os << "[sub ";
	
	if (nuc)
		os << nuc << ' ';
	else
		os << "[par]";

	if (d)
		os << down().data_ << ']';
	if (u) 
		os << up().data_ << ']';
}


void MathScriptInset::maplize2(MathInset const * nuc, MapleStream & os) const
{
	if (nuc)
		os << nuc;
	if (hasDown() && down().data_.size())
		os << '[' << down().data_ << ']';
	if (hasUp() && up().data_.size())
		os << "^(" << up().data_ << ')';
}


void MathScriptInset::mathmlize2(MathInset const * nuc, MathMLStream & os) const
{
	bool d = hasDown() && down().data_.size();
	bool u = hasUp() && up().data_.size();

	if (u && d)
		os << MTag("msubsup");
	else if (u)
		os << MTag("msup");
	else if (d)
		os << MTag("msub");

	if (nuc)
		os << nuc;
	else
		os << "<mrow/>";

	if (u && d)
		os << down().data_ << up().data_ << ETag("msubsup");
	else if (u)
		os << up().data_ << ETag("msup");
	else if (d)
		os << down().data_ << ETag("msub");
}


void MathScriptInset::octavize2(MathInset const * nuc, OctaveStream & os) const
{
	if (nuc)
		os << nuc;
	if (hasDown() && down().data_.size())
		os << '[' << down().data_ << ']';
	if (hasUp() && up().data_.size())
		os << "^(" << up().data_ << ')';
}


