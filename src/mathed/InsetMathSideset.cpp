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

InsetMathSideset::InsetMathSideset(Buffer * buf)
	: InsetMathNest(buf, 5)
{}


InsetMathSideset::InsetMathSideset(Buffer * buf, MathAtom const & at)
	: InsetMathNest(buf, 5)
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
	int dasc = max(bl().dimension(bv).ascent(), br().dimension(bv).ascent());
	int slevel = nuc().slevel();
	int ascdrop = dasc - slevel;
	int desdrop = isCharBox ? 0 : des + nuc().sshift();
	int mindes = nuc().mindes();
	des = max(desdrop, ascdrop);
	des = max(mindes, des);
	int minasc = nuc().minasc();
	ascdrop = isCharBox ? 0 : asc - min(tl().mindes(), tr().mindes());
	int udes = max(bl().dimension(bv).descent(), tr().dimension(bv).descent());
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
	int des = max(bl().dimension(bv).ascent(), br().dimension(bv).ascent());
	int na = nasc(bv);
	des = dybt(bv, na, nd, false);
	return des;
}


int InsetMathSideset::dyt(BufferView const & bv) const
{
	int na = nasc(bv);
	int asc = max(tl().dimension(bv).descent(), tr().dimension(bv).descent());
	int nd = ndes(bv);
	asc = dybt(bv, na, nd, true);
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
	ScriptChanger dummy(mi.base);
	bl().metrics(mi, dimbl);
	tl().metrics(mi, dimtl);
	br().metrics(mi, dimbr);
	tr().metrics(mi, dimtr);

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
	ScriptChanger dummy(pi.base);
	bl().draw(pi, x          , y + dyb(bv));
	tl().draw(pi, x          , y - dyt(bv));
	br().draw(pi, x + dxr(bv), y + dyb(bv));
	tr().draw(pi, x + dxr(bv), y - dyt(bv));
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



bool InsetMathSideset::idxForward(Cursor &) const
{
	return false;
}


bool InsetMathSideset::idxBackward(Cursor &) const
{
	return false;
}


bool InsetMathSideset::idxUpDown(Cursor & cur, bool up) const
{
	// in nucleus?
	if (cur.idx() == 0) {
		// go up/down only if in the last position
		// or in the first position
		if (cur.pos() == cur.lastpos() || cur.pos() == 0) {
			if (cur.pos() == 0)
				cur.idx() = up ? 2 : 1;
			else
				cur.idx() = up ? 4 : 3;
			cur.pos() = 0;
			return true;
		}
		return false;
	}

	// Are we 'up'?
	if (cur.idx() == 2 || cur.idx() == 4) {
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
	if (cur.idx() == 1 || cur.idx() == 3) {
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
	for (int i = 0; i < 2; ++i) {
		os << '{';
		if (!cell(2*i+1).empty())
			os << "_{" << cell(2*i+1) << '}';
		if (!cell(2*i+2).empty())
			os << "^{" << cell(2*i+2) << '}';
		os << '}';
	}
	os << '{' << nuc() << '}';

	if (lock_ && !os.latex())
		os << "\\lyxlock ";
}


void InsetMathSideset::normalize(NormalStream & os) const
{
	os << "[sideset ";

	if (!bl().empty())
		os << bl() << ' ';
	if (!tl().empty())
		os << tl() << ' ';

	if (!nuc().empty())
		os << nuc() << ' ';
	else
		os << "[par]";

	if (!br().empty())
		os << br() << ' ';
	if (!tr().empty())
		os << tr() << ' ';
	os << ']';
}


void InsetMathSideset::mathmlize(MathStream & os) const
{
	os << MTag("mmultiscripts");

	if (nuc().empty())
		os << "<mrow />";
	else
		os << MTag("mrow") << nuc() << ETag("mrow");

	if (br().empty())
		os << "<none />";
	else
		os << MTag("mrow") << br() << ETag("mrow");
	if (tr().empty())
		os << "<none />";
	else
		os << MTag("mrow") << tr() << ETag("mrow");

	if (bl().empty())
		os << "<none />";
	else
		os << MTag("mrow") << bl() << ETag("mrow");
	if (tl().empty())
		os << "<none />";
	else
		os << MTag("mrow") << tl() << ETag("mrow");

	os << ETag("mmultiscripts");
}


void InsetMathSideset::htmlize(HtmlStream & os) const
{
	bool const havebl = !bl().empty();
	bool const havetl = !tl().empty();
	bool const havebr = !br().empty();
	bool const havetr = !tr().empty();

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
