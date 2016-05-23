/**
 * \file InsetMathSideset.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BufferView.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetMathSideset.h"
#include "InsetMathSymbol.h"
#include "LaTeXFeatures.h"
#include "MathData.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "support/debug.h"

#include "support/lassert.h"


using namespace std;


namespace {
	/// x spacing between the nucleus and the scripts
	int const dx = 2;
}


namespace lyx {

InsetMathSideset::InsetMathSideset(Buffer * buf, bool scriptl, bool scriptr)
	: InsetMathNest(buf, 3 + scriptl + scriptr), scriptl_(scriptl),
	  scriptr_(scriptr)
{}


InsetMathSideset::InsetMathSideset(Buffer * buf, bool scriptl, bool scriptr,
                                   MathAtom const & at)
	: InsetMathNest(buf, 3 + scriptl + scriptr), scriptl_(scriptl),
	  scriptr_(scriptr)
{
	nuc().push_back(at);
}


Inset * InsetMathSideset::clone() const
{
	return new InsetMathSideset(*this);
}


bool InsetMathSideset::idxFirst(Cursor & cur) const
{
	cur.idx() = 0;
	cur.pos() = 0;
	return true;
}


bool InsetMathSideset::idxLast(Cursor & cur) const
{
	cur.idx() = 0;
	cur.pos() = nuc().size();
	return true;
}


int InsetMathSideset::dybt(BufferView const & bv, int asc, int des, bool top) const
{
	bool isCharBox = nuc().empty() ? false : isAlphaSymbol(nuc().back());
	int dasc = 0;
	if (scriptl_ && scriptr_)
		dasc = max(bl().dimension(bv).ascent(), br().dimension(bv).ascent());
	else if (scriptl_)
		dasc = bl().dimension(bv).ascent();
	else if (scriptr_)
		dasc = br().dimension(bv).ascent();
	int slevel = nuc().slevel();
	int ascdrop = dasc - slevel;
	int desdrop = isCharBox ? 0 : des + nuc().sshift();
	int mindes = nuc().mindes();
	des = max(desdrop, ascdrop);
	des = max(mindes, des);
	int minasc = nuc().minasc();
	ascdrop = 0;
	if (!isCharBox && (scriptl_ || scriptr_)) {
		if (scriptl_ && scriptr_)
			ascdrop = asc - min(tl().mindes(), tr().mindes());
		else if (scriptl_)
			ascdrop = asc - tl().mindes();
		else if (scriptr_)
			ascdrop = asc - tr().mindes();
	}
	int udes = 0;
	if (scriptl_)
		udes = bl().dimension(bv).descent();
	if (scriptr_)
		udes = max(udes, br().dimension(bv).descent());
	asc = udes + nuc().sshift();
	asc = max(ascdrop, asc);
	asc = max(minasc, asc);
	int del = asc - udes - dasc;
	if (del + des <= 2) {
		int newdes = 2 - del;
		del = slevel - asc + udes;
		if (del > 0) {
			asc += del;
			newdes -= del;
		}
		des = max(des, newdes);
	}
	return top ? asc : des;
}


int InsetMathSideset::dyb(BufferView const & bv) const
{
	int nd = ndes(bv);
	int na = nasc(bv);
	int des = dybt(bv, na, nd, false);
	return des;
}


int InsetMathSideset::dyt(BufferView const & bv) const
{
	int na = nasc(bv);
	int nd = ndes(bv);
	int asc = dybt(bv, na, nd, true);
	return asc;
}


int InsetMathSideset::dxr(BufferView const & bv) const
{
	return dxn(bv) + nwid(bv) + dx;
}


int InsetMathSideset::dxn(BufferView const & bv) const
{
	Dimension const dimb = bl().dimension(bv);
	Dimension const dimt = tl().dimension(bv);
	return max(dimb.width(), dimt.width()) + dx;
}


int InsetMathSideset::nwid(BufferView const & bv) const
{
	return nuc().dimension(bv).width();
}


int InsetMathSideset::nasc(BufferView const & bv) const
{
	return nuc().dimension(bv).ascent();
}


int InsetMathSideset::ndes(BufferView const & bv) const
{
	return nuc().dimension(bv).descent();
}


int InsetMathSideset::nker(BufferView const * bv) const
{
	int const kerning = nuc().kerning(bv);
	return max(kerning, 0);
}


void InsetMathSideset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Dimension dimn;
	Dimension dimbl;
	Dimension dimtl;
	Dimension dimbr;
	Dimension dimtr;
	nuc().metrics(mi, dimn);
	if (!scriptl_) {
		bl().metrics(mi, dimbl);
		dimtl = dimbl;
	}
	if (!scriptr_) {
		br().metrics(mi, dimbr);
		dimtr = dimbr;
	}
	Changer dummy = mi.base.changeScript();
	if (scriptl_) {
		bl().metrics(mi, dimbl);
		tl().metrics(mi, dimtl);
	}
	if (scriptr_) {
		br().metrics(mi, dimbr);
		tr().metrics(mi, dimtr);
	}

	BufferView & bv = *mi.base.bv;
	// FIXME: data copying... not very efficient.

	dim.wid = nwid(bv) + nker(mi.base.bv) + 2 * dx;
	dim.wid += max(dimbl.width(), dimtl.width());
	dim.wid += max(dimbr.width(), dimtr.width());
	int na = nasc(bv);
	int asc = dyt(bv) + max(dimtl.ascent(), dimtr.ascent());
	dim.asc = max(na, asc);
	int nd = ndes(bv);
	int des = dyb(bv) + max(dimbl.descent(), dimbr.descent());
	dim.des = max(nd, des);
	metricsMarkers(dim);
}


void InsetMathSideset::draw(PainterInfo & pi, int x, int y) const
{
	BufferView & bv = *pi.base.bv;
	nuc().draw(pi, x + dxn(bv), y);
	if (!scriptl_)
		bl().draw(pi, x          , y);
	if (!scriptr_)
		br().draw(pi, x + dxr(bv), y);
	Changer dummy = pi.base.changeScript();
	if (scriptl_) {
		bl().draw(pi, x          , y + dyb(bv));
		tl().draw(pi, x          , y - dyt(bv));
	}
	if (scriptr_) {
		br().draw(pi, x + dxr(bv), y + dyb(bv));
		tr().draw(pi, x + dxr(bv), y - dyt(bv));
	}
	drawMarkers(pi, x, y);
}


void InsetMathSideset::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	bl().metricsT(mi, dim);
	tl().metricsT(mi, dim);
	br().metricsT(mi, dim);
	tr().metricsT(mi, dim);
	nuc().metricsT(mi, dim);
}


void InsetMathSideset::drawT(TextPainter & pain, int x, int y) const
{
	// FIXME: BROKEN
	nuc().drawT(pain, x + 1, y);
	bl().drawT(pain, x + 1, y + 1 /*dy0()*/);
	tl().drawT(pain, x + 1, y - 1 /*dy1()*/);
	br().drawT(pain, x + 1, y + 1 /*dy0()*/);
	tr().drawT(pain, x + 1, y - 1 /*dy1()*/);
}



bool InsetMathSideset::idxForward(Cursor & cur) const
{
	if (!scriptl_ && cur.idx() == 1) {
		// left => nucleus
		cur.idx() = 0;
		return true;
	} else if (!scriptr_ && cur.idx() == 0) {
		// nucleus => right
		cur.idx() = 2 + scriptl_;
		return true;
	}
	return false;
}


bool InsetMathSideset::idxBackward(Cursor & cur) const
{
	if (!scriptr_ && cur.idx() == (scriptl_ ? 3 : 2)) {
		// right => nucleus
		cur.idx() = 0;
		return true;
	} else if (!scriptl_ && cur.idx() == 0) {
		// nucleus => left
		cur.idx() = 1;
		return true;
	}
	return false;
}


bool InsetMathSideset::idxUpDown(Cursor & cur, bool up) const
{
	// in nucleus?
	if (cur.idx() == 0) {
		// go up/down only if in the last position
		// or in the first position
		if ((scriptr_ && cur.pos() == cur.lastpos()) ||
		    (scriptl_ && cur.pos() == 0)) {
			if (cur.pos() == 0)
				cur.idx() = up ? 2 : 1;
			else
				cur.idx() = (up ? 3 : 2) + scriptl_;
			cur.pos() = 0;
			return true;
		}
		return false;
	}

	// Are we 'up'?
	if ((scriptl_ && cur.idx() == 2) ||
	    (scriptr_ && cur.idx() == (scriptl_ ? 4 : 3))) {
		// can't go further up
		if (up)
			return false;
		// otherwise go to first or last position in the nucleus
		cur.idx() = 0;
		if (cur.idx() == 2)
			cur.pos() = 0;
		else
			cur.pos() = cur.lastpos();
		return true;
	}

	// Are we 'down'?
	if ((scriptl_ && cur.idx() == 1) ||
	    (scriptr_ && cur.idx() == (scriptl_ ? 3 : 2))) {
		// can't go further down
		if (!up)
			return false;
		// otherwise go to first or last position in the nucleus
		cur.idx() = 0;
		if (cur.idx() == 1)
			cur.pos() = 0;
		else
			cur.pos() = cur.lastpos();
		return true;
	}

	return false;
}


void InsetMathSideset::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);

	os << "\\sideset";
	os << '{';
	if (scriptl_) {
		if (!bl().empty())
			os << "_{" << bl() << '}';
		if (!tl().empty())
			os << "^{" << tl() << '}';
	} else
		os << bl();
	os << "}{";
	if (scriptr_) {
		if (!br().empty())
			os << "_{" << br() << '}';
		if (!tr().empty())
			os << "^{" << tr() << '}';
	} else
		os << br();
	os << '}';
	os << nuc();

	if (lock_ && !os.latex())
		os << "\\lyxlock ";
}


void InsetMathSideset::normalize(NormalStream & os) const
{
	os << "[sideset ";

	if (!bl().empty())
		os << bl() << ' ';
	if (scriptl_ && !tl().empty())
		os << tl() << ' ';

	if (!nuc().empty())
		os << nuc() << ' ';
	else
		os << "[par]";

	if (!br().empty())
		os << br() << ' ';
	if (scriptr_ && !tr().empty())
		os << tr() << ' ';
	os << ']';
}


void InsetMathSideset::mathmlize(MathStream & os) const
{
	// FIXME This is only accurate if both scriptl_ and scriptr_ are true
	if (!scriptl_)
		os << MTag("mrow") << bl() << ETag("mrow");
	if (scriptl_ || scriptr_) {
		os << MTag("mmultiscripts");

		if (nuc().empty())
			os << "<mrow />";
		else
			os << MTag("mrow") << nuc() << ETag("mrow");

		if (br().empty() || !scriptr_)
			os << "<none />";
		else
			os << MTag("mrow") << br() << ETag("mrow");
		if (tr().empty() || !scriptr_)
			os << "<none />";
		else
			os << MTag("mrow") << tr() << ETag("mrow");

		if (bl().empty() || !scriptl_)
			os << "<none />";
		else
			os << MTag("mrow") << bl() << ETag("mrow");
		if (tl().empty() || !scriptl_)
			os << "<none />";
		else
			os << MTag("mrow") << tl() << ETag("mrow");

		os << ETag("mmultiscripts");
	}
	if (!scriptr_)
		os << MTag("mrow") << br() << ETag("mrow");
}


void InsetMathSideset::htmlize(HtmlStream & os) const
{
	// FIXME This is only accurate if both scriptl_ and scriptr_ are true
	bool const havebl = scriptl_ && !bl().empty();
	bool const havetl = scriptl_ && !tl().empty();
	bool const havebr = scriptr_ && !br().empty();
	bool const havetr = scriptr_ && !tr().empty();

	if (!scriptl_ && !bl().empty())
		os << bl();

	if (havebl && havetl)
		os << MTag("span", "class='scripts'")
			 << MTag("span") << tl() << ETag("span")
			 << MTag("span") << bl() << ETag("span")
			 << ETag("span");
	else if (havebl)
		os << MTag("sub", "class='math'") << bl() << ETag("sub");
	else if (havetl)
		os << MTag("sup", "class='math'") << tl() << ETag("sup");

	if (!nuc().empty())
		os << nuc();

	if (havebr && havetr)
		os << MTag("span", "class='scripts'")
			 << MTag("span") << tr() << ETag("span")
			 << MTag("span") << br() << ETag("span")
			 << ETag("span");
	else if (havebr)
		os << MTag("sub", "class='math'") << br() << ETag("sub");
	else if (havetr)
		os << MTag("sup", "class='math'") << tr() << ETag("sup");

	if (!scriptr_ && !br().empty())
		os << br();
}


void InsetMathSideset::infoize(odocstream & os) const
{
	os << "Sideset";
}


// the idea for dual scripts came from the eLyXer code
void InsetMathSideset::validate(LaTeXFeatures & features) const
{
	if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		features.addCSSSnippet(
			"span.scripts{display: inline-block; vertical-align: middle; text-align:center; font-size: 75%;}\n"
			"span.scripts span {display: block;}\n"
			"sub.math{font-size: 75%;}\n"
			"sup.math{font-size: 75%;}");
	features.require("amsmath");
	InsetMathNest::validate(features);
}

} // namespace lyx
