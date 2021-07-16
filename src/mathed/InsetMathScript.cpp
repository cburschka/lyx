/**
 * \file InsetMathScript.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathScript.h"

#include "InsetMathBrace.h"
#include "MathData.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "BufferView.h"
#include "Cursor.h"
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lassert.h"


using namespace std;

namespace lyx {


InsetMathScript::InsetMathScript(Buffer * buf)
	: InsetMathNest(buf, 1), cell_1_is_up_(false)
{}


InsetMathScript::InsetMathScript(Buffer * buf, bool up)
	: InsetMathNest(buf, 2), cell_1_is_up_(up)
{}


InsetMathScript::InsetMathScript(Buffer * buf, MathAtom const & at, bool up)
	: InsetMathNest(buf, 2), cell_1_is_up_(up)
{
	LATTEST(nargs() >= 1);
	cell(0).push_back(at);
}


Inset * InsetMathScript::clone() const
{
	return new InsetMathScript(*this);
}


InsetMathScript const * InsetMathScript::asScriptInset() const
{
	return this;
}


InsetMathScript * InsetMathScript::asScriptInset()
{
	return this;
}


MathData const & InsetMathScript::down() const
{
	if (nargs() == 3)
		return cell(2);
	LBUFERR(nargs() > 1);
	return cell(1);
}


MathData & InsetMathScript::down()
{
	if (nargs() == 3)
		return cell(2);
	LBUFERR(nargs() > 1);
	return cell(1);
}


MathData const & InsetMathScript::up() const
{
	LBUFERR(nargs() > 1);
	return cell(1);
}


MathData & InsetMathScript::up()
{
	LBUFERR(nargs() > 1);
	return cell(1);
}


void InsetMathScript::ensure(bool up)
{
	if (nargs() == 1) {
		// just nucleus so far
		cells_.push_back(MathData());
		cell_1_is_up_ = up;
	} else if (nargs() == 2 && !has(up)) {
		if (up) {
			cells_.push_back(cell(1));
			cell(1).clear();
		} else {
			cells_.push_back(MathData());
		}
	}
}


MathData const & InsetMathScript::nuc() const
{
	return cell(0);
}


MathData & InsetMathScript::nuc()
{
	return cell(0);
}


int InsetMathScript::dy01(BufferView const & bv, int asc, int des, int what) const
{
	int dasc = 0;
	int slevel = 0;
	bool isCharBox = !nuc().empty() ? isAlphaSymbol(nuc().back()) : false;
	if (hasDown()) {
		Dimension const & dimdown = down().dimension(bv);
		dasc = dimdown.ascent();
		slevel = nuc().slevel();
		int ascdrop = dasc - slevel;
		int desdrop = isCharBox ? 0 : des + nuc().sshift();
		int mindes = nuc().mindes();
		des = max(desdrop, ascdrop);
		des = max(mindes, des);
	}
	if (hasUp()) {
		Dimension const & dimup = up().dimension(bv);
		int minasc = nuc().minasc();
		int ascdrop = isCharBox ? 0 : asc - up().mindes();
		int udes = dimup.descent();
		asc = udes + nuc().sshift();
		asc = max(ascdrop, asc);
		asc = max(minasc, asc);
		if (hasDown()) {
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
		}
	}
	return what ? asc : des;
}


int InsetMathScript::dy0(BufferView const & bv) const
{
	int nd = ndes(bv);
	if (!hasDown())
		return nd;
	int des = down().dimension(bv).ascent();
	if (has_limits_)
		des += nd + 2;
	else {
		int na = nasc(bv);
		des = dy01(bv, na, nd, 0);
	}
	return des;
}


int InsetMathScript::dy1(BufferView const & bv) const
{
	int na = nasc(bv);
	if (!hasUp())
		return na;
	int asc = up().dimension(bv).descent();
	if (has_limits_)
		asc += na + 2;
	else {
		int nd = ndes(bv);
		asc = dy01(bv, na, nd, 1);
	}
	asc = max(asc, 5);
	return asc;
}


int InsetMathScript::dx0(BufferView const & bv) const
{
	LASSERT(hasDown(), return 0);
	Dimension const dim = dimension(bv);
	return has_limits_ ? (dim.wid - down().dimension(bv).width()) / 2
		: nwid(bv) + min(nker(&bv), 0);
}


int InsetMathScript::dx1(BufferView const & bv) const
{
	LASSERT(hasUp(), return 0);
	Dimension const dim = dimension(bv);
	return has_limits_ ? (dim.wid - up().dimension(bv).width()) / 2
		: nwid(bv) + max(nker(&bv), 0);
}


int InsetMathScript::dxx(BufferView const & bv) const
{
	Dimension const dim = dimension(bv);
	return has_limits_ ? (dim.wid - nwid(bv)) / 2  :  0;
}


int InsetMathScript::nwid(BufferView const & bv) const
{
	return !nuc().empty() ? nuc().dimension(bv).width() : 2;
}


int InsetMathScript::nasc(BufferView const & bv) const
{
	return !nuc().empty() ? nuc().dimension(bv).ascent() : 5;
}


int InsetMathScript::ndes(BufferView const & bv) const
{
	return !nuc().empty() ? nuc().dimension(bv).descent() : 0;
}


int InsetMathScript::nker(BufferView const * bv) const
{
	if (!nuc().empty())
		return nuc().kerning(bv);
	return 0;
}


Limits InsetMathScript::limits() const
{
	if (nuc().empty())
		return AUTO_LIMITS;
	else
		// only the limits status of the last element counts
		return nuc().back()->limits();
}


void InsetMathScript::limits(Limits lim)
{
	if (!nuc().empty())
		nuc().back()->limits(lim);
}


MathClass InsetMathScript::mathClass() const
{
	// FIXME: this is a hack, since the class will not be correct if
	// the nucleus has several elements or if the last element is a math macro
	// or a macro argument proxy.
	// The correct implementation would require to linearize the nucleus.
	if (nuc().empty())
		return MC_ORD;
	else {
		// return the class of last element since this is the one that counts.
		MathClass mc = nuc().back()->mathClass();
		return (mc == MC_UNKNOWN) ? MC_ORD : mc;
	}
}


void InsetMathScript::metrics(MetricsInfo & mi, Dimension & dim) const
{
	// we store this, because it is much easier
	has_limits_ = hasLimits(mi.base.font.style());

	// Compute metrics of the available cells
	Dimension dim0;
	Dimension dim1;
	Dimension dim2;
	{
		// Keeps the changers local
		Changer dummy2 = mi.base.changeEnsureMath();
		cell(0).metrics(mi, dim0);
		Changer dummy = mi.base.changeScript();
		if (nargs() > 1)
			cell(1).metrics(mi, dim1, !has_limits_);
		if (nargs() > 2)
			cell(2).metrics(mi, dim2, !has_limits_);
	}

	dim.wid = 0;
	BufferView & bv = *mi.base.bv;
	// FIXME: data copying... not very efficient.
	Dimension dimup;
	Dimension dimdown;
	if (hasUp())
		dimup = up().dimension(bv);
	if (hasDown())
		dimdown = down().dimension(bv);

	if (has_limits_) {
		dim.wid = nwid(bv);
		if (hasUp())
			dim.wid = max(dim.wid, dimup.width());
		if (hasDown())
			dim.wid = max(dim.wid, dimdown.width());
	} else {
		if (hasUp())
			dim.wid = max(dim.wid, max(nker(mi.base.bv), 0) + dimup.width());
		if (hasDown())
			dim.wid = max(dim.wid, min(nker(mi.base.bv), 0) + dimdown.width());
		dim.wid += nwid(bv);
	}
	int na = nasc(bv);
	if (hasUp()) {
		int asc = dy1(bv) + dimup.ascent();
		dim.asc = max(na, asc);
	} else
		dim.asc = na;
	int nd = ndes(bv);
	if (hasDown()) {
		int des = dy0(bv) + dimdown.descent();
		dim.des = max(nd, des);
	} else
		dim.des = nd;
}


void InsetMathScript::draw(PainterInfo & pi, int x, int y) const
{
	Changer dummy2 = pi.base.changeEnsureMath();
	BufferView & bv = *pi.base.bv;
	if (!nuc().empty())
		nuc().draw(pi, x + dxx(bv), y);
	else {
		nuc().setXY(bv, x + dxx(bv), y);
		if (editing(&bv))
			pi.draw(x + dxx(bv), y, char_type('.'));
	}
	Changer dummy = pi.base.changeScript();
	if (hasUp())
		up().draw(pi, x + dx1(bv), y - dy1(bv));
	if (hasDown())
		down().draw(pi, x + dx0(bv), y + dy0(bv));
}


void InsetMathScript::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	if (hasUp())
		up().metricsT(mi, dim);
	if (hasDown())
		down().metricsT(mi, dim);
	nuc().metricsT(mi, dim);
}


void InsetMathScript::drawT(TextPainter & pain, int x, int y) const
{
	// FIXME: BROKEN
	if (!nuc().empty())
		nuc().drawT(pain, x + 1, y);
	if (hasUp())
		up().drawT(pain, x + 1, y - 1 /*dy1()*/);
	if (hasDown())
		down().drawT(pain, x + 1, y + 1 /*dy0()*/);
}


bool InsetMathScript::hasLimits(MathStyle const & style) const
{
	if (nuc().empty())
		return false;

	Limits const lim = nuc().back()->limits() == AUTO_LIMITS
		? nuc().back()->defaultLimits(style == DISPLAY_STYLE)
		: nuc().back()->limits();
	LASSERT(lim != AUTO_LIMITS, return false);
	return lim == LIMITS;
}


void InsetMathScript::removeScript(bool up)
{
	if (nargs() == 2) {
		if (up == cell_1_is_up_)
			cells_.pop_back();
	} else if (nargs() == 3) {
		if (up) {
			swap(cells_[1], cells_[2]);
			cell_1_is_up_ = false;
		} else {
			cell_1_is_up_ = true;
		}
		cells_.pop_back();
	}
}


bool InsetMathScript::has(bool up) const
{
	return idxOfScript(up);
}


bool InsetMathScript::hasUp() const
{
	//lyxerr << "1up: " << bool(cell_1_is_up_));
	//lyxerr << "hasUp: " << bool(idxOfScript(true)));
	return idxOfScript(true);
}


bool InsetMathScript::hasDown() const
{
	//LYXERR0("1up: " << bool(cell_1_is_up_));
	//LYXERR0("hasDown: " << bool(idxOfScript(false)));
	return idxOfScript(false);
}


idx_type InsetMathScript::idxOfScript(bool up) const
{
	if (nargs() == 1)
		return 0;
	if (nargs() == 2)
		return (cell_1_is_up_ == up) ? 1 : 0;
	if (nargs() == 3)
		return up ? 1 : 2;
	LASSERT(false, return 0);
}


bool InsetMathScript::idxForward(Cursor &) const
{
	return false;
}


bool InsetMathScript::idxBackward(Cursor &) const
{
	return false;
}


bool InsetMathScript::idxUpDown(Cursor & cur, bool up) const
{
	// in nucleus?
	if (cur.idx() == 0) {
		// don't go up/down if there is no cell in this direction
		if (!has(up))
			return false;
		// go up/down only if in the last position
		// or in the first position of something with displayed limits
		if (cur.pos() == cur.lastpos()
		                 || (cur.pos() == 0 && has_limits_)) {
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


void InsetMathScript::write(TeXMathStream & os) const
{
	MathEnsurer ensurer(os);

	if (!nuc().empty()) {
		os << nuc();
		// Avoid double superscript errors (bug 1633)
		if (os.latex() && hasUp() && nuc().back()->getChar() == '\'')
			os << "{}";
	} else if (os.firstitem())
		LYXERR(Debug::MATHED, "suppressing {} when writing");
	else
		os << "{}";

	if (hasDown() /*&& !down().empty()*/)
		os << "_{" << down() << '}';

	if (hasUp() /*&& !up().empty()*/) {
		// insert space if up() is empty or an empty brace inset
		// (see bug 8305)
		if (os.latex() && (up().empty() ||
		    (up().size() == 1 && up().back()->asBraceInset() &&
		     up().back()->asBraceInset()->cell(0).empty())))
			os << "^ {}";
		else {
			os << "^{" << up() << '}';
			// Avoid double superscript errors by writing an
			// empty group {} when a prime immediately follows
			if (os.latex())
				os.useBraces(true);
		}
	}

	if (lock_ && !os.latex())
		os << "\\lyxlock ";
}


void InsetMathScript::normalize(NormalStream & os) const
{
	bool d = hasDown() && !down().empty();
	bool u = hasUp() && !up().empty();

	if (u && d)
		os << "[subsup ";
	else if (u)
		os << "[sup ";
	else if (d)
		os << "[sub ";

	if (!nuc().empty())
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


void InsetMathScript::maple(MapleStream & os) const
{
	if (!nuc().empty())
		os << nuc();
	if (hasDown() && !down().empty())
		os << '[' << down() << ']';
	if (hasUp() && !up().empty())
		os << "^(" << up() << ')';
}


void InsetMathScript::mathematica(MathematicaStream & os) const
{
	bool d = hasDown() && !down().empty();
	bool u = hasUp() && !up().empty();

	if (!nuc().empty()) {
		if (d)
			os << "Subscript[" << nuc();
		else
			os << nuc();
	}

	if (u)
		os << "^(" << up() << ')';

	if (!nuc().empty()) {
		if (d)
			os << ',' << down() << ']';
	}
}


void InsetMathScript::mathmlize(MathMLStream & ms) const
{
	bool d = hasDown() && !down().empty();
	bool u = hasUp() && !up().empty();
	bool has_limits = hasLimits(ms.getFontMathStyle());

	if (!d && !u)
		return;

	const char * tag = nullptr;
	if (u && d)
		tag = has_limits ? "munderover" : "msubsup";
	else if (u)
		tag = has_limits ? "mover" : "msup";
	else if (d)
		tag = has_limits ? "munder" : "msub";

	ms << MTag(tag);

	if (!nuc().empty())
		ms << nuc();
	else
		ms << CTag("mrow");

	if (d)
		ms << MTag("mrow") << down() << ETag("mrow");
	if (u)
		ms << MTag("mrow") << up() << ETag("mrow");

	ms << ETag(tag);
}


void InsetMathScript::htmlize(HtmlStream & os) const
{
	bool d = hasDown() && !down().empty();
	bool u = hasUp() && !up().empty();

	if (!nuc().empty())
		os << nuc();

	if (u && d)
		os << MTag("span", "class='scripts'")
			 << MTag("span") << up() << ETag("span")
			 << MTag("span") << down() << ETag("span")
			 << ETag("span");
	else if (u)
		os << MTag("sup", "class='math'") << up() << ETag("sup");
	else if (d)
		os << MTag("sub", "class='math'") << down() << ETag("sub");
}


void InsetMathScript::octave(OctaveStream & os) const
{
	if (!nuc().empty())
		os << nuc();
	if (hasDown() && !down().empty())
		os << '[' << down() << ']';
	if (hasUp() && !up().empty())
		os << "^(" << up() << ')';
}


void InsetMathScript::infoize(odocstream & os) const
{
	os << "Scripts";
}


void InsetMathScript::infoize2(odocstream & os) const
{
	os << from_ascii(has_limits_ == 1 ? ", Displayed limits" : ", Inlined limits");
}


bool InsetMathScript::notifyCursorLeaves(Cursor const & old, Cursor & cur)
{
	InsetMathNest::notifyCursorLeaves(old, cur);

	//LYXERR0("InsetMathScript::notifyCursorLeaves: 1 " << cur);

	// Remove empty scripts if possible:

	// The case of two scripts, but only one got empty (1 = super, 2 = sub).
	// We keep the script inset, but remove the empty script.
	if (nargs() > 2 && (!cell(1).empty() || !cell(2).empty())) {
		if (cell(2).empty()) {
			// must be a subscript...
			old.recordUndoInset();
			removeScript(false);
			cur.screenUpdateFlags(cur.result().screenUpdate() | Update::SinglePar);
			return true;
		} else if (cell(1).empty()) {
			// must be a superscript...
			old.recordUndoInset();
			removeScript(true);
			cur.screenUpdateFlags(cur.result().screenUpdate() | Update::SinglePar);
			return true;
		}
	}
	// Now the two suicide cases:
	// * we have only one script which is empty
	// * we have two scripts which are both empty.
	// The script inset is removed completely.
	if ((nargs() == 2 && cell(1).empty())
	    || (nargs() == 3 && cell(1).empty() && cell(2).empty())) {
		// Make undo step. We cannot use cur for this because
		// it does not necessarily point to us anymore. But we
		// should be on top of the cursor old.
		Cursor insetCur = old;
		int scriptSlice	= insetCur.find(this);
		LASSERT(scriptSlice != -1, /**/);
		insetCur.cutOff(scriptSlice);
		insetCur.recordUndoInset();

		// Let the script inset commit suicide. This is
		// modelled on Cursor.pullArg(), but tries not to
		// invoke notifyCursorLeaves again and does not touch
		// cur (since the top slice will be deleted
		// afterwards)
		MathData ar = cell(0);
		insetCur.pop();
		insetCur.cell().erase(insetCur.pos());
		insetCur.cell().insert(insetCur.pos(), ar);

		// redraw
		cur.screenUpdateFlags(cur.result().screenUpdate() | Update::SinglePar);
		return true;
	}

	//LYXERR0("InsetMathScript::notifyCursorLeaves: 2 " << cur);
	return false;
}


// the idea for dual scripts came from the eLyXer code
void InsetMathScript::validate(LaTeXFeatures & features) const
{
	if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		features.addCSSSnippet(
			"span.scripts{display: inline-block; vertical-align: middle; text-align:center; font-size: 75%;}\n"
			"span.scripts span {display: block;}\n"
			"sub.math{font-size: 75%;}\n"
			"sup.math{font-size: 75%;}");
	InsetMathNest::validate(features);
}

} // namespace lyx
