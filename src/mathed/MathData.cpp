/**
 * \file MathData.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MathData.h"

#include "InsetMathBrace.h"
#include "InsetMathFont.h"
#include "InsetMathScript.h"
#include "MacroTable.h"
#include "MathMacro.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "MetricsInfo.h"
#include "ReplaceData.h"

#include "Buffer.h"
#include "BufferView.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "debug.h"

#include "support/docstream.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include <boost/assert.hpp>
#include <boost/next_prior.hpp>


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



void MathData::metrics(MetricsInfo & mi, Dimension & dim) const
{
	frontend::FontMetrics const & fm = theFontMetrics(mi.base.font);
	dim = fm.dimension('I');
	int xascent = fm.dimension('x').ascent();
	if (xascent >= dim.asc)
		xascent = (2 * dim.asc) / 3;
	minasc_ = xascent;
	mindes_ = (3 * xascent) / 4;
	slevel_ = (4 * xascent) / 5;
	sshift_ = xascent / 4;
	kerning_ = 0;

	if (empty()) {
		// Cache the dimension.
		mi.base.bv->coordCache().arrays().add(this, dim);
		return;
	}

	const_cast<MathData*>(this)->updateMacros(mi);

	dim.asc = 0;
	dim.wid = 0;
	Dimension d;
	atom_dims_.clear();
	for (size_t i = 0, n = size(); i != n; ++i) {
		MathAtom const & at = operator[](i);
		at->metrics(mi, d);
		atom_dims_.push_back(d);
		dim += d;
		if (i == n - 1)
			kerning_ = at->kerning();
	}
	// Cache the dimension.
	mi.base.bv->coordCache().arrays().add(this, dim);
}


void MathData::draw(PainterInfo & pi, int x, int y) const
{
	//lyxerr << "MathData::draw: x: " << x << " y: " << y << endl;
	BufferView & bv  = *pi.base.bv;
	setXY(bv, x, y);

	Dimension const & dim = bv.coordCache().getArrays().dim(this);

	if (empty()) {
		pi.pain.rectangle(x, y - dim.ascent(), dim.width(), dim.height(), Color_mathline);
		return;
	}

	// don't draw outside the workarea
	if (y + dim.descent() <= 0
		|| y - dim.ascent() >= bv.workHeight()
		|| x + dim.width() <= 0
		|| x >= bv. workWidth())
		return;

	for (size_t i = 0, n = size(); i != n; ++i) {
		MathAtom const & at = operator[](i);
		bv.coordCache().insets().add(at.nucleus(), x, y);
		at->drawSelection(pi, x, y);
		at->draw(pi, x, y);
		x += atom_dims_[i].wid;
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


void MathData::updateMacros(MetricsInfo & mi) 
{
	Cursor & cur = mi.base.bv->cursor();

	// go over the array and look for macros
	for (size_t i = 0; i < size(); ++i) {
		MathMacro * macroInset = operator[](i).nucleus()->asMacro();
		if (!macroInset)
			continue;
		
		// get macro
		macroInset->updateMacro(mi);
		size_t macroNumArgs = 0;
		size_t macroOptionals = 0;
		MacroData const * macro = macroInset->macro();
		if (macro) {
			macroNumArgs = macro->numargs();
			macroOptionals = macro->optionals();
		}

		// store old and compute new display mode
		MathMacro::DisplayMode newDisplayMode;
		MathMacro::DisplayMode oldDisplayMode = macroInset->displayMode();
		newDisplayMode = macroInset->computeDisplayMode(mi);

		// arity changed or other reason to detach?
		if (oldDisplayMode == MathMacro::DISPLAY_NORMAL
				&& (macroInset->arity() != macroNumArgs
						|| macroInset->optionals() != macroOptionals
						|| newDisplayMode == MathMacro::DISPLAY_UNFOLDED)) {
			detachMacroParameters(cur, i);
		}

		// the macro could have been copied while resizing this
		macroInset = operator[](i).nucleus()->asMacro();

		// Cursor in \label?
		if (newDisplayMode != MathMacro::DISPLAY_UNFOLDED 
				&& oldDisplayMode == MathMacro::DISPLAY_UNFOLDED) {
			// put cursor in front of macro
			int macroSlice = cur.find(macroInset);
			if (macroSlice != -1)
				cur.cutOff(macroSlice - 1);
		}

		// update the display mode
		macroInset->setDisplayMode(newDisplayMode);

		// arity changed?
		if (newDisplayMode == MathMacro::DISPLAY_NORMAL 
				&& (macroInset->arity() != macroNumArgs
						|| macroInset->optionals() != macroOptionals)) {
			// is it a virgin macro which was never attached to parameters?
			bool fromInitToNormalMode
			= (oldDisplayMode == MathMacro::DISPLAY_INIT 
				 || oldDisplayMode == MathMacro::DISPLAY_NONGREEDY_INIT)
				&& newDisplayMode == MathMacro::DISPLAY_NORMAL;
			bool greedy = (oldDisplayMode != MathMacro::DISPLAY_NONGREEDY_INIT);
			
			// attach parameters
			attachMacroParameters(cur, i, macroNumArgs, macroOptionals,
				fromInitToNormalMode, greedy);
			
			// FIXME: proper anchor handling, this removes the selection
			cur.updateInsets(&cur.bottom().inset());
			cur.clearSelection();	
		}

		// give macro the chance to adapt to new situation
		InsetMath * inset = operator[](i).nucleus();
		if (inset->asScriptInset())
			inset = inset->asScriptInset()->nuc()[0].nucleus();
		BOOST_ASSERT(inset->asMacro());
		inset->asMacro()->updateRepresentation(mi);
	}
}


void MathData::detachMacroParameters(Cursor & cur, const size_type macroPos)
{
	MathMacro * macroInset = operator[](macroPos).nucleus()->asMacro();
	
	// detach all arguments
	std::vector<MathData> detachedArgs;
	if (macroPos + 1 == size())
				// strip arguments if we are at the MathData end
				macroInset->detachArguments(detachedArgs, true);
	else
				macroInset->detachArguments(detachedArgs, false);
	
	// find cursor slice
	int curMacroSlice = cur.find(macroInset);
	idx_type curMacroIdx = -1;
	pos_type curMacroPos = -1;
	std::vector<CursorSlice> argSlices;
	if (curMacroSlice != -1) {
				curMacroPos = cur[curMacroSlice].pos();
				curMacroIdx = cur[curMacroSlice].idx();
				cur.cutOff(curMacroSlice, argSlices);
				cur.pop_back();
	}
	
	// only [] after the last non-empty argument can be dropped later 
	size_t lastNonEmptyOptional = 0;
	for (size_t l = 0; l < detachedArgs.size() && l < macroInset->optionals(); ++l) {
				if (!detachedArgs[l].empty())
					lastNonEmptyOptional = l;
	}
	
	// optional arguments to be put back?
	pos_type p = macroPos + 1;
	size_t j = 0;
	for (; j < detachedArgs.size() && j < macroInset->optionals(); ++j) {
		// another non-empty parameter follows?
		bool canDropEmptyOptional = j >= lastNonEmptyOptional;
		
		// then we can drop empty optional parameters
		if (detachedArgs[j].empty() && canDropEmptyOptional) {
			if (curMacroIdx == j)
				cur[curMacroSlice - 1].pos() = macroPos + 1;
			continue;
		}
		
		// Otherwise we don't drop an empty optional, put it back normally
		MathData optarg;
		asArray(from_ascii("[]"), optarg);
		MathData & arg = detachedArgs[j];
		
		// look for "]", i.e. put a brace around?
		InsetMathBrace * brace = 0;
		for (size_t q = 0; q < arg.size(); ++q) {
			if (arg[q]->getChar() == ']') {
				// put brace
				brace = new InsetMathBrace();
				break;
			}
		}
		
		// put arg between []
		if (brace) {
			brace->cell(0) = arg;
			optarg.insert(1, MathAtom(brace));
		} else
			optarg.insert(1, arg);
		
		// insert it into the array
		insert(p, optarg);
		p += optarg.size();
		
		// cursor in optional argument of macro?
		if (curMacroIdx == j) {
			if (brace) {
				cur.append(0, curMacroPos);
				cur[curMacroSlice - 1].pos() = macroPos + 2;
			} else
				cur[curMacroSlice - 1].pos() = macroPos + 2 + curMacroPos;
			cur.append(argSlices);
		} else if (cur[curMacroSlice - 1].pos() >= int(p))
			// cursor right of macro
			cur[curMacroSlice - 1].pos() += optarg.size();
	}
	
	// put them back into the MathData
	for (; j < detachedArgs.size(); ++j) {				
		MathData const & arg = detachedArgs[j];
		if (arg.size() == 1 && !arg[0]->asScriptInset()) // && arg[0]->asCharInset())
			insert(p, arg[0]);
		else
			insert(p, MathAtom(new InsetMathBrace(arg)));
		
		// cursor in j-th argument of macro?
		if (curMacroIdx == j) {
			if (operator[](p).nucleus()->asBraceInset()) {
				cur[curMacroSlice - 1].pos() = p;
				cur.append(0, curMacroPos);
				cur.append(argSlices);
			} else {
				cur[curMacroSlice - 1].pos() = p; // + macroPos;
				cur.append(argSlices);
			}
		} else if (cur[curMacroSlice - 1].pos() >= int(p))
			++cur[curMacroSlice - 1].pos();
		
		++p;
	}
	
	// FIXME: proper anchor handling, this removes the selection
	cur.clearSelection();
	cur.updateInsets(&cur.bottom().inset());
}


void MathData::attachMacroParameters(Cursor & cur, 
	const size_type macroPos, const size_type macroNumArgs,
	const int macroOptionals, const bool fromInitToNormalMode,
	const bool greedy)
{
	MathMacro * macroInset = operator[](macroPos).nucleus()->asMacro();

	// start at atom behind the macro again, maybe with some new arguments from above
	// to add them back into the macro inset
	size_t p = macroPos + 1;
	std::vector<MathData> detachedArgs;
	MathAtom scriptToPutAround;
	
	// find cursor slice again
	int thisSlice = cur.find(*this);
	int thisPos = -1;
	if (thisSlice != -1)
		thisPos = cur[thisSlice].pos();
	
	// find arguments behind the macro
	if (greedy) {
		collectOptionalParameters(cur, macroOptionals, detachedArgs, p,
			macroPos, thisPos, thisSlice);
		collectParameters(cur, macroNumArgs, detachedArgs, p,
			scriptToPutAround, 
			macroPos, thisPos, thisSlice);
	}
		
	// attach arguments back to macro inset
	macroInset->attachArguments(detachedArgs, macroNumArgs, macroOptionals);
	
	// found tail script? E.g. \foo{a}b^x
	if (scriptToPutAround.nucleus()) {
		// put macro into a script inset
		scriptToPutAround.nucleus()->asScriptInset()->nuc()[0] 
		= operator[](macroPos);
		operator[](macroPos) = scriptToPutAround;
		
		if (thisPos == int(macroPos))
			cur.append(0, 0);
	}
	
	// remove them from the MathData
	erase(begin() + macroPos + 1, begin() + p);

	// fix up cursor
	if (thisSlice != -1) {
		// fix cursor if right of p
		if (thisPos >= int(p))
			cur[thisSlice].pos() -= p - (macroPos + 1);
	
		// was the macro inset just inserted and was now folded?
		if (cur[thisSlice].pos() == int(macroPos + 1)
				&& fromInitToNormalMode
				&& macroInset->arity() > 0
				&& thisSlice + 1 == int(cur.depth())) {
			// then enter it if the cursor was just behind
			cur[thisSlice].pos() = macroPos;
			cur.push_back(CursorSlice(*macroInset));
			macroInset->idxFirst(cur);
		}
	}
}


void MathData::collectOptionalParameters(Cursor & cur, 
	const size_type numOptionalParams, std::vector<MathData> & params, 
	size_t & pos, const pos_type macroPos, const int thisPos, const int thisSlice)
{
	// insert optional arguments?
	while (params.size() < numOptionalParams && pos < size()) {
		// is a [] block following which could be an optional parameter?
		if (operator[](pos)->getChar() != '[')
			break;
				
		// found possible optional argument, look for "]"
		size_t right = pos + 1;
		for (; right < size(); ++right) {
			if (operator[](right)->getChar() == ']')
				// found right end
				break;
		}
		
		// found?
		if (right >= size()) {
			// no ] found, so it's not an optional argument
			break;
		}
		
		// add everything between [ and ] as optional argument
		MathData optarg(begin() + pos + 1, begin() + right);
		
		// a brace?
		bool brace = false;
		if (optarg.size() == 1 && optarg[0]->asBraceInset()) {
			brace = true;
			params.push_back(optarg[0]->asBraceInset()->cell(0));
		} else
			params.push_back(optarg);
		
		// place cursor in optional argument of macro
		if (thisSlice != -1
				&& thisPos >= int(pos) && thisPos <= int(right)) {
			int paramPos = std::max(0, thisPos - int(pos) - 1);
			std::vector<CursorSlice> x;
			cur.cutOff(thisSlice, x);
			cur[thisSlice].pos() = macroPos;
			if (brace) {
				paramPos = x[0].pos();
				x.erase(x.begin());
			}
			cur.append(0, paramPos);
			cur.append(x);
		}
		pos = right + 1;
	}

	// fill up empty optional parameters
	while (params.size() < numOptionalParams) {
		params.push_back(MathData());
	}
}


void MathData::collectParameters(Cursor & cur, 
	const size_type numParams, std::vector<MathData> & params, 
	size_t & pos, MathAtom & scriptToPutAround,
	const pos_type macroPos, const int thisPos, const int thisSlice) 
{
	// insert normal arguments
	for (; params.size() < numParams && pos < size();) {
		MathAtom & cell = operator[](pos);
		
		// fix cursor
		std::vector<CursorSlice> argSlices;
		int argPos = 0;
		if (thisSlice != -1 && thisPos == int(pos)) {
			cur.cutOff(thisSlice, argSlices);
		}
		
		// which kind of parameter is it? In {}? With index x^n?
		InsetMathBrace const * brace = cell->asBraceInset();
		if (brace) {
			// found brace, convert into argument
			params.push_back(brace->cell(0));
			
			// cursor inside of the brace or just in front of?
			if (thisPos == int(pos) && !argSlices.empty()) {
				argPos = argSlices[0].pos();
				argSlices.erase(argSlices.begin());
			}
		} else if (cell->asScriptInset() && params.size() + 1 == numParams) {
			// last inset with scripts without braces
			// -> they belong to the macro, not the argument
			InsetMathScript * script = cell.nucleus()->asScriptInset();
			if (script->nuc().size() == 1 && script->nuc()[0]->asBraceInset())
				// nucleus in brace? Unpack!
				params.push_back(script->nuc()[0]->asBraceInset()->cell(0));
			else
				params.push_back(script->nuc());
			
			// script will be put around below
			scriptToPutAround = cell;
			
			// this should only happen after loading, so make cursor handling simple
			if (thisPos >= int(macroPos) && thisPos <= int(macroPos + numParams)) {
				argSlices.clear();
				cur.append(0, 0);
			}
		} else {
			// the simplest case: plain inset
			MathData array;
			array.insert(0, cell);
			params.push_back(array);
		}
		
		// put cursor in argument again
		if (thisSlice != - 1 && thisPos == int(pos)) {
			cur.append(params.size() - 1, argPos);
			cur.append(argSlices);
			cur[thisSlice].pos() = macroPos;
		}
		
		++pos;
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
		x += atom_dims_[i].wid;
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
		currx += atom_dims_[it - begin()].wid;
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
	return bv.coordCache().getArrays().squareDistance(this, x, y);
}


void MathData::setXY(BufferView & bv, int x, int y) const
{
	//lyxerr << "setting position cache for MathData " << this << std::endl;
	bv.coordCache().arrays().add(this, x, y);
}


Dimension const & MathData::dimension(BufferView const & bv) const
{
	return bv.coordCache().getArrays().dim(this);
}


int MathData::xm(BufferView const & bv) const
{
	Geometry const & g = bv.coordCache().getArrays().geometry(this);

	return g.pos.x_ + g.dim.wid / 2;
}


int MathData::ym(BufferView const & bv) const
{
	Geometry const & g = bv.coordCache().getArrays().geometry(this);

	return g.pos.y_ + (g.dim.des - g.dim.asc) / 2;
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
