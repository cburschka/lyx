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
#include "InsetMathMacro.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "MetricsInfo.h"
#include "ReplaceData.h"

#include "Buffer.h"
#include "BufferView.h"
#include "CoordCache.h"
#include "Cursor.h"

#include "mathed/InsetMathUnknown.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lyxalgo.h"

#include <cstdlib>

using namespace std;

namespace lyx {


MathData::MathData(Buffer * buf, const_iterator from, const_iterator to)
	: base_type(from, to), minasc_(0), mindes_(0), slevel_(0),
	  sshift_(0), kerning_(0), buffer_(buf)
{}


MathAtom & MathData::operator[](pos_type pos)
{
	LBUFERR(pos < size());
	return base_type::operator[](pos);
}


MathAtom const & MathData::operator[](pos_type pos) const
{
	LBUFERR(pos < size());
	return base_type::operator[](pos);
}


void MathData::insert(size_type pos, MathAtom const & t)
{
	LBUFERR(pos <= size());
	base_type::insert(begin() + pos, t);
}


void MathData::insert(size_type pos, MathData const & ar)
{
	LBUFERR(pos <= size());
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


bool MathData::addToMathRow(MathRow & mrow, MetricsInfo & mi) const
{
	bool has_contents = false;
	BufferView * bv = mi.base.bv;
	MathData * ar = const_cast<MathData*>(this);
	ar->updateMacros(&bv->cursor(), mi.macrocontext,
	                 InternalUpdate, mi.base.macro_nesting);


	// FIXME: for completion, try to insert the relevant data in the
	// mathrow (like is done for text rows). We could add a pair of
	// InsetMathColor inset, but these come with extra spacing of
	// their own.
	DocIterator const & inlineCompletionPos = bv->inlineCompletionPos();
	bool const has_completion = inlineCompletionPos.inMathed()
		&& &inlineCompletionPos.cell() == this;
	size_t const compl_pos = has_completion ? inlineCompletionPos.pos() : 0;

	for (size_t i = 0 ; i < size() ; ++i) {
		has_contents |= (*this)[i]->addToMathRow(mrow, mi);
		if (i + 1 == compl_pos) {
			mrow.back().compl_text = bv->inlineCompletion();
			mrow.back().compl_unique_to = bv->inlineCompletionUniqueChars();
		}
	}
	return has_contents;
}


#if 0
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
#endif


bool MathData::hasCaret(BufferView * bv) const
{
	Cursor & cur = bv->cursor();
	return cur.inMathed() && &cur.cell() == this;
}


void MathData::metrics(MetricsInfo & mi, Dimension & dim, bool tight) const
{
	frontend::FontMetrics const & fm = theFontMetrics(mi.base.font);
	BufferView * bv = mi.base.bv;
	int const Iascent = fm.dimension('I').ascent();
	int xascent = fm.dimension('x').ascent();
	if (xascent >= Iascent)
		xascent = (2 * Iascent) / 3;
	minasc_ = xascent;
	mindes_ = (3 * xascent) / 4;
	slevel_ = (4 * xascent) / 5;
	sshift_ = xascent / 4;

	MathRow mrow(mi, this);
	bool has_caret = mrow.metrics(mi, dim);
	mrow_cache_[bv] = mrow;
	kerning_ = mrow.kerning(bv);

	// Set a minimal ascent/descent for the cell
	if (tight)
		// FIXME: this is the minimal ascent seen empirically, check
		// what the TeXbook says.
		dim.asc = max(dim.asc, fm.ascent('x'));
	else {
		dim.asc = max(dim.asc, fm.maxAscent());
		dim.des = max(dim.des, fm.maxDescent());
	}

	// This is one of the the few points where the drawing font is known,
	// so that we can set the caret vertical dimensions.
	has_caret |= hasCaret(bv);
	if (has_caret)
		bv->setCaretAscentDescent(min(dim.asc, fm.maxAscent()),
		                          min(dim.des, fm.maxDescent()));

	// Cache the dimension.
	bv->coordCache().arrays().add(this, dim);
}


void MathData::drawSelection(PainterInfo & pi, int const x, int const y) const
{
	BufferView const * bv = pi.base.bv;
	Cursor const & cur = bv->cursor();
	InsetMath const * inset = cur.inset().asInsetMath();
	if (!cur.selection() || !inset || inset->nargs() == 0)
		return;

	CursorSlice const s1 = cur.selBegin();
	CursorSlice const s2 = cur.selEnd();
	MathData const & c1 = inset->cell(s1.idx());

	if (s1.idx() == s2.idx() && &c1 == this) {
		// selection indide cell
		Dimension const dim = bv->coordCache().getArrays().dim(&c1);
		int const beg = c1.pos2x(bv, s1.pos());
		int const end = c1.pos2x(bv, s2.pos());
		pi.pain.fillRectangle(x + beg, y - dim.ascent(),
		                      end - beg, dim.height(), Color_selection);
	} else {
		for (idx_type i = 0; i < inset->nargs(); ++i) {
			MathData const & c = inset->cell(i);
			if (&c == this && inset->idxBetween(i, s1.idx(), s2.idx())) {
				// The whole cell is selected
				Dimension const dim = bv->coordCache().getArrays().dim(&c);
				pi.pain.fillRectangle(x, y - dim.ascent(),
				                      dim.width(), dim.height(),
				                      Color_selection);
			}
		}
	}
}


void MathData::draw(PainterInfo & pi, int const x, int const y) const
{
	//lyxerr << "MathData::draw: x: " << x << " y: " << y << endl;
	setXY(*pi.base.bv, x, y);

	drawSelection(pi, x, y);
	MathRow const & mrow = mrow_cache_[pi.base.bv];
	mrow.draw(pi, x, y);
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


void MathData::updateBuffer(ParIterator const & it, UpdateType utype)
{
	// pass down
	for (size_t i = 0, n = size(); i != n; ++i) {
		MathAtom & at = operator[](i);
		at.nucleus()->updateBuffer(it, utype);
	}
}


void MathData::updateMacros(Cursor * cur, MacroContext const & mc,
		UpdateType utype, int nesting)
{
	// If we are editing a macro, we cannot update it immediately,
	// otherwise wrong undo steps will be recorded (bug 6208).
	InsetMath const * inmath = cur ? cur->inset().asInsetMath() : 0;
	InsetMathMacro const * inmacro = inmath ? inmath->asMacro() : 0;
	docstring const edited_name = inmacro ? inmacro->name() : docstring();

	// go over the array and look for macros
	for (size_t i = 0; i < size(); ++i) {
		InsetMathMacro * macroInset = operator[](i).nucleus()->asMacro();
		if (!macroInset || macroInset->macroName().empty()
				|| macroInset->macroName()[0] == '^'
				|| macroInset->macroName()[0] == '_'
				|| (macroInset->name() == edited_name
				    && macroInset->displayMode() ==
						InsetMathMacro::DISPLAY_UNFOLDED))
			continue;

		// get macro
		macroInset->updateMacro(mc);
		size_t macroNumArgs = 0;
		size_t macroOptionals = 0;
		MacroData const * macro = macroInset->macro();
		if (macro) {
			macroNumArgs = macro->numargs();
			macroOptionals = macro->optionals();
		}

		// store old and compute new display mode
		InsetMathMacro::DisplayMode newDisplayMode;
		InsetMathMacro::DisplayMode oldDisplayMode = macroInset->displayMode();
		newDisplayMode = macroInset->computeDisplayMode();

		// arity changed or other reason to detach?
		if (oldDisplayMode == InsetMathMacro::DISPLAY_NORMAL
		    && (macroInset->arity() != macroNumArgs
			|| macroInset->optionals() != macroOptionals
			|| newDisplayMode == InsetMathMacro::DISPLAY_UNFOLDED))
			detachMacroParameters(cur, i);

		// the macro could have been copied while resizing this
		macroInset = operator[](i).nucleus()->asMacro();

		// Cursor in \label?
		if (newDisplayMode != InsetMathMacro::DISPLAY_UNFOLDED
		    && oldDisplayMode == InsetMathMacro::DISPLAY_UNFOLDED) {
			// put cursor in front of macro
			if (cur) {
				int macroSlice = cur->find(macroInset);
				if (macroSlice != -1)
					cur->cutOff(macroSlice - 1);
			}
		}

		// update the display mode
		size_t appetite = macroInset->appetite();
		macroInset->setDisplayMode(newDisplayMode);

		// arity changed?
		if (newDisplayMode == InsetMathMacro::DISPLAY_NORMAL
		    && (macroInset->arity() != macroNumArgs
			|| macroInset->optionals() != macroOptionals)) {
			// is it a virgin macro which was never attached to parameters?
			bool fromInitToNormalMode
			= (oldDisplayMode == InsetMathMacro::DISPLAY_INIT
			   || oldDisplayMode == InsetMathMacro::DISPLAY_INTERACTIVE_INIT)
			  && newDisplayMode == InsetMathMacro::DISPLAY_NORMAL;

			// if the macro was entered interactively (i.e. not by paste or during
			// loading), it should not be greedy, but the cursor should
			// automatically jump into the macro when behind
			bool interactive = (oldDisplayMode == InsetMathMacro::DISPLAY_INTERACTIVE_INIT);

			// attach parameters
			attachMacroParameters(cur, i, macroNumArgs, macroOptionals,
				fromInitToNormalMode, interactive, appetite);

			if (cur)
				cur->updateInsets(&cur->bottom().inset());
		}

		// Give macro the chance to adapt to new situation.
		// The macroInset could be invalid now because it was put into a script
		// inset and therefore "deep" copied. So get it again from the MathData.
		InsetMath * inset = operator[](i).nucleus();
		if (inset->asScriptInset())
			inset = inset->asScriptInset()->nuc()[0].nucleus();
		LASSERT(inset->asMacro(), continue);
		inset->asMacro()->updateRepresentation(cur, mc, utype, nesting + 1);
	}
}


void MathData::detachMacroParameters(DocIterator * cur, const size_type macroPos)
{
	InsetMathMacro * macroInset = operator[](macroPos).nucleus()->asMacro();
	// We store this now, because the inset pointer will be invalidated in the scond loop below
	size_t const optionals = macroInset->optionals();

	// detach all arguments
	vector<MathData> detachedArgs;
	if (macroPos + 1 == size())
		// strip arguments if we are at the MathData end
		macroInset->detachArguments(detachedArgs, true);
	else
		macroInset->detachArguments(detachedArgs, false);

	// find cursor slice
	int curMacroSlice = -1;
	if (cur)
		curMacroSlice = cur->find(macroInset);
	idx_type curMacroIdx = -1;
	pos_type curMacroPos = -1;
	vector<CursorSlice> argSlices;
	if (curMacroSlice != -1) {
		curMacroPos = (*cur)[curMacroSlice].pos();
		curMacroIdx = (*cur)[curMacroSlice].idx();
		cur->cutOff(curMacroSlice, argSlices);
		cur->pop_back();
	}

	// only [] after the last non-empty argument can be dropped later
	size_t lastNonEmptyOptional = 0;
	for (size_t l = 0; l < detachedArgs.size() && l < optionals; ++l) {
		if (!detachedArgs[l].empty())
			lastNonEmptyOptional = l;
	}

	// optional arguments to be put back?
	pos_type p = macroPos + 1;
	size_t j = 0;
	// We do not want to use macroInset below, the insert() call in
	// the loop will invalidate it.
	macroInset = 0;
	for (; j < detachedArgs.size() && j < optionals; ++j) {
		// another non-empty parameter follows?
		bool canDropEmptyOptional = j >= lastNonEmptyOptional;

		// then we can drop empty optional parameters
		if (detachedArgs[j].empty() && canDropEmptyOptional) {
			if (curMacroIdx == j)
				(*cur)[curMacroSlice - 1].pos() = macroPos + 1;
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
				brace = new InsetMathBrace(buffer_);
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

		// cursor in macro?
		if (curMacroSlice == -1)
			continue;

		// cursor in optional argument of macro?
		if (curMacroIdx == j) {
			if (brace) {
				cur->append(0, curMacroPos);
				(*cur)[curMacroSlice - 1].pos() = macroPos + 2;
			} else
				(*cur)[curMacroSlice - 1].pos() = macroPos + 2 + curMacroPos;
			cur->append(argSlices);
		} else if ((*cur)[curMacroSlice - 1].pos() >= int(p))
			// cursor right of macro
			(*cur)[curMacroSlice - 1].pos() += optarg.size();
	}

	// put them back into the MathData
	for (; j < detachedArgs.size(); ++j, ++p) {
		MathData const & arg = detachedArgs[j];
		if (arg.size() == 1
		    && !arg[0]->asScriptInset()
		    && !(arg[0]->asMacro() && arg[0]->asMacro()->arity() > 0))
			insert(p, arg[0]);
		else
			insert(p, MathAtom(new InsetMathBrace(arg)));

		// cursor in macro?
		if (curMacroSlice == -1)
			continue;

		// cursor in j-th argument of macro?
		if (curMacroIdx == j) {
			if (operator[](p).nucleus()->asBraceInset()) {
				(*cur)[curMacroSlice - 1].pos() = p;
				cur->append(0, curMacroPos);
				cur->append(argSlices);
			} else {
				(*cur)[curMacroSlice - 1].pos() = p; // + macroPos;
				cur->append(argSlices);
			}
		} else if ((*cur)[curMacroSlice - 1].pos() >= int(p))
			++(*cur)[curMacroSlice - 1].pos();
	}

	if (cur)
		cur->updateInsets(&cur->bottom().inset());
}


void MathData::attachMacroParameters(Cursor * cur,
	const size_type macroPos, const size_type macroNumArgs,
	const int macroOptionals, const bool fromInitToNormalMode,
	const bool interactiveInit, const size_t appetite)
{
	InsetMathMacro * macroInset = operator[](macroPos).nucleus()->asMacro();

	// start at atom behind the macro again, maybe with some new arguments
	// from the detach phase above, to add them back into the macro inset
	size_t p = macroPos + 1;
	vector<MathData> detachedArgs;
	MathAtom scriptToPutAround;

	// find cursor slice again of this MathData
	int thisSlice = -1;
	if (cur)
		thisSlice = cur->find(*this);
	int thisPos = -1;
	if (thisSlice != -1)
		thisPos = (*cur)[thisSlice].pos();

	// find arguments behind the macro
	if (!interactiveInit) {
		collectOptionalParameters(cur, macroOptionals, detachedArgs, p,
			scriptToPutAround, macroPos, thisPos, thisSlice);
	}
	collectParameters(cur, macroNumArgs, detachedArgs, p,
		scriptToPutAround, macroPos, thisPos, thisSlice, appetite);

	// attach arguments back to macro inset
	macroInset->attachArguments(detachedArgs, macroNumArgs, macroOptionals);

	// found tail script? E.g. \foo{a}b^x
	if (scriptToPutAround.nucleus()) {
		InsetMathScript * scriptInset =
			scriptToPutAround.nucleus()->asScriptInset();
		// In the math parser we remove empty braces in the base
		// of a script inset, but we have to restore them here.
		if (scriptInset->nuc().empty()) {
			MathData ar;
			scriptInset->nuc().push_back(
					MathAtom(new InsetMathBrace(ar)));
		}
		// put macro into a script inset
		scriptInset->nuc()[0] = operator[](macroPos);
		operator[](macroPos) = scriptToPutAround;

		// go into the script inset nucleus
		if (cur && thisPos == int(macroPos))
			cur->append(0, 0);

		// get pointer to "deep" copied macro inset
		scriptInset = operator[](macroPos).nucleus()->asScriptInset();
		macroInset = scriptInset->nuc()[0].nucleus()->asMacro();
	}

	// remove them from the MathData
	erase(macroPos + 1, p);

	// cursor outside this MathData?
	if (thisSlice == -1)
		return;

	// fix cursor if right of p
	if (thisPos >= int(p))
		(*cur)[thisSlice].pos() -= p - (macroPos + 1);

	// was the macro inset just inserted interactively and was now folded
	// and the cursor is just behind?
	if ((*cur)[thisSlice].pos() == int(macroPos + 1)
	    && interactiveInit
	    && fromInitToNormalMode
	    && macroInset->arity() > 0
	    && thisSlice + 1 == int(cur->depth())) {
		// then enter it if the cursor was just behind
		(*cur)[thisSlice].pos() = macroPos;
		cur->push_back(CursorSlice(*macroInset));
		macroInset->idxFirst(*cur);
	}
}


void MathData::collectOptionalParameters(Cursor * cur,
	const size_type numOptionalParams, vector<MathData> & params,
	size_t & pos, MathAtom & scriptToPutAround,
	const pos_type macroPos, const int thisPos, const int thisSlice)
{
	Buffer * buf = cur ? cur->buffer() : 0;
	// insert optional arguments?
	while (params.size() < numOptionalParams
	       && pos < size()
	       && !scriptToPutAround.nucleus()) {
		// is a [] block following which could be an optional parameter?
		if (operator[](pos)->getChar() != '[')
			break;

		// found possible optional argument, look for pairing "]"
		int count = 1;
		size_t right = pos + 1;
		for (; right < size(); ++right) {
			MathAtom & cell = operator[](right);

			if (cell->getChar() == '[')
				++count;
			else if (cell->getChar() == ']' && --count == 0)
				// found right end
				break;

			// maybe "]" with a script around?
			InsetMathScript * script = cell.nucleus()->asScriptInset();
			if (!script)
				continue;
			if (script->nuc().size() != 1)
				continue;
			if (script->nuc()[0]->getChar() == ']') {
				// script will be put around the macro later
				scriptToPutAround = cell;
				break;
			}
		}

		// found?
		if (right >= size()) {
			// no ] found, so it's not an optional argument
			break;
		}

		// add everything between [ and ] as optional argument
		MathData optarg(buf, begin() + pos + 1, begin() + right);

		// a brace?
		bool brace = false;
		if (optarg.size() == 1 && optarg[0]->asBraceInset()) {
			brace = true;
			params.push_back(optarg[0]->asBraceInset()->cell(0));
		} else
			params.push_back(optarg);

		// place cursor in optional argument of macro
		// Note: The two expressions on the first line are equivalent
		// (see caller), but making this explicit pleases coverity.
		if (cur && thisSlice != -1
		    && thisPos >= int(pos) && thisPos <= int(right)) {
			int paramPos = max(0, thisPos - int(pos) - 1);
			vector<CursorSlice> x;
			cur->cutOff(thisSlice, x);
			(*cur)[thisSlice].pos() = macroPos;
			if (brace) {
				paramPos = x[0].pos();
				x.erase(x.begin());
			}
			cur->append(0, paramPos);
			cur->append(x);
		}
		pos = right + 1;
	}

	// fill up empty optional parameters
	while (params.size() < numOptionalParams)
		params.push_back(MathData());
}


void MathData::collectParameters(Cursor * cur,
	const size_type numParams, vector<MathData> & params,
	size_t & pos, MathAtom & scriptToPutAround,
	const pos_type macroPos, const int thisPos, const int thisSlice,
	const size_t appetite)
{
	size_t startSize = params.size();

	// insert normal arguments
	while (params.size() < numParams
	       && params.size() - startSize < appetite
	       && pos < size()
	       && !scriptToPutAround.nucleus()) {
		MathAtom & cell = operator[](pos);

		// fix cursor
		vector<CursorSlice> argSlices;
		int argPos = 0;
		// Note: The two expressions on the first line are equivalent
		// (see caller), but making this explicit pleases coverity.
		if (cur && thisSlice != -1
			&& thisPos == int(pos))
			cur->cutOff(thisSlice, argSlices);

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
				if (cur)
					cur->append(0, 0);
			}
		} else {
			// the simplest case: plain inset
			MathData array;
			array.insert(0, cell);
			params.push_back(array);
		}

		// put cursor in argument again
		// Note: The first two expressions on the first line are
		// equivalent (see caller), but making this explicit pleases
		// coverity.
		if (cur && thisSlice != -1 && thisPos == int(pos)) {
			cur->append(params.size() - 1, argPos);
			cur->append(argSlices);
			(*cur)[thisSlice].pos() = macroPos;
		}

		++pos;
	}
}


int MathData::pos2x(BufferView const * bv, size_type pos) const
{
	int x = 0;
	size_type target = min(pos, size());
	CoordCache::Insets const & coords = bv->coordCache().getInsets();
	for (size_type i = 0; i < target; ++i) {
		const_iterator it = begin() + i;
		//lyxerr << "char: " << (*it)->getChar()
		//	<< "width: " << (*it)->width() << endl;
		x += coords.dim((*it).nucleus()).wid;
	}
	return x;
}


MathData::size_type MathData::x2pos(BufferView const * bv, int targetx) const
{
	const_iterator it = begin();
	int lastx = 0;
	int currx = 0;
	CoordCache::Insets const & coords = bv->coordCache().getInsets();
	// find first position after targetx
	for (; currx < targetx && it != end(); ++it) {
		lastx = currx;
		currx += coords.dim((*it).nucleus()).wid;
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
	    && ((*prev(it, 1))->asNestInset()
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
	//lyxerr << "setting position cache for MathData " << this << endl;
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


MathClass MathData::mathClass() const
{
	MathClass res = MC_UNKNOWN;
	for (MathAtom const & at : *this) {
		MathClass mc = at->mathClass();
		if (res == MC_UNKNOWN)
			res = mc;
		else if (mc != MC_UNKNOWN && res != mc)
			return MC_ORD;
	}
	return res == MC_UNKNOWN ? MC_ORD : res;
}


ostream & operator<<(ostream & os, MathData const & ar)
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
