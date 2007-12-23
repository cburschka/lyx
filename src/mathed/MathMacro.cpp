/**
 * \file MathMacro.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MathMacro.h"
#include "MathSupport.h"
#include "MathExtern.h"
#include "MathStream.h"

#include "Buffer.h"
#include "BufferView.h"
#include "Cursor.h"
#include "support/debug.h"
#include "LaTeXFeatures.h"
#include "FuncStatus.h"
#include "FuncRequest.h"
#include "Undo.h"

#include "frontends/Painter.h"

#include <ostream>
#include <vector>

using namespace std;

namespace lyx {


/// A proxy for the macro values
class ArgumentProxy : public InsetMath {
public:
	///
	ArgumentProxy(MathMacro & mathMacro, size_t idx) 
		: mathMacro_(mathMacro), idx_(idx) {}
	///
	ArgumentProxy(MathMacro & mathMacro, size_t idx, docstring const & def) 
		: mathMacro_(mathMacro), idx_(idx) 
	{
			asArray(def, def_);
	}
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const {
		mathMacro_.macro()->unlock();
		mathMacro_.cell(idx_).metrics(mi, dim);
		if (!mathMacro_.editing(mi.base.bv) && !def_.empty())
			def_.metrics(mi, dim);
		mathMacro_.macro()->lock();
	}
	///
	void draw(PainterInfo & pi, int x, int y) const {
		if (mathMacro_.editing(pi.base.bv)) {
			// The only way a ArgumentProxy can appear is in a cell of the 
			// MathMacro. Moreover the cells are only drawn in the DISPLAY_FOLDED 
			// mode and then, in the case of "editing_ == true" the monochrome 
			// mode is entered by the MathMacro before calling the cells' draw
			// method. Then eventually this code is reached and the proxy leaves
			// monochrome mode temporarely. Hence, if it is not in monochrome 
			// here (and the assert triggers in pain.leaveMonochromeMode()) 
			// it's a bug.
			pi.pain.leaveMonochromeMode();
			mathMacro_.cell(idx_).draw(pi, x, y);
			pi.pain.enterMonochromeMode(Color_mathbg, Color_mathmacroblend);
		} else {
			if (def_.empty())
				mathMacro_.cell(idx_).draw(pi, x, y);
			else {
				mathMacro_.cell(idx_).setXY(*pi.base.bv, x, y);
				def_.draw(pi, x, y);
			}
		}
	}
	///
	size_t idx() const { return idx_; }
	///
	int kerning(BufferView const * bv) const { return mathMacro_.cell(idx_).kerning(bv); }

private:
	///
	Inset * clone() const 
	{
		return new ArgumentProxy(*this);
	}
	///
	MathMacro & mathMacro_;
	///
	size_t idx_;
	///
	MathData def_;
};


MathMacro::MathMacro(docstring const & name)
	: InsetMathNest(0), name_(name), displayMode_(DISPLAY_INIT),
		attachedArgsNum_(0), optionals_(0), nextFoldMode_(true),
		macro_(0), needsUpdate_(false)
{}


Inset * MathMacro::clone() const
{
	MathMacro * copy = new MathMacro(*this);
	copy->needsUpdate_ = true;
	copy->expanded_.cell(0).clear();
	return copy;
}


docstring MathMacro::name() const
{
	if (displayMode_ == DISPLAY_UNFOLDED)
		return asString(cell(0));
	else
		return name_;
}


void MathMacro::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x,	int & y) const
{
	// We may have 0 arguments, but InsetMathNest requires at least one.
	if (nargs() > 0)
		InsetMathNest::cursorPos(bv, sl, boundary, x, y);
}


int MathMacro::cursorIdx(Cursor const & cur) const {
	for (size_t i = 0; i != cur.depth(); ++i)
			if (&cur[i].inset() == this)
				return cur[i].idx();
	return -1;
}


bool MathMacro::editMode(Cursor const & cur) const {
	// find this in cursor trace
	for (size_t i = 0; i != cur.depth(); ++i)
		if (&cur[i].inset() == this) {
			// look if there is no other macro in edit mode above
			++i;
			for (; i != cur.depth(); ++i) {
				MathMacro const * macro = dynamic_cast<MathMacro const *>(&cur[i].inset());
				if (macro && macro->displayMode() == DISPLAY_NORMAL)
					return false;
			}

			// ok, none found, I am the highest one
			return true;
		}

	return false;
}


void MathMacro::metrics(MetricsInfo & mi, Dimension & dim) const
{
	// set edit mode for which we will have calculated metrics
	editing_[mi.base.bv] = editMode(mi.base.bv->cursor());

	// calculate new metrics according to display mode
	if (displayMode_ == DISPLAY_INIT || displayMode_ == DISPLAY_INTERACTIVE_INIT) {
		mathed_string_dim(mi.base.font, from_ascii("\\") + name(), dim);
	} else if (displayMode_ == DISPLAY_UNFOLDED) {
		cell(0).metrics(mi, dim);
		Dimension bsdim;
		mathed_string_dim(mi.base.font, from_ascii("\\"), bsdim);
		dim.wid += bsdim.width() + 1;
		dim.asc = max(bsdim.ascent(), dim.ascent());
		dim.des = max(bsdim.descent(), dim.descent());
		metricsMarkers(dim);
	} else {
		BOOST_ASSERT(macro_ != 0);

		// calculate metric finally
		macro_->lock();
		expanded_.cell(0).metrics(mi, dim);
		macro_->unlock();

		// calculate dimension with label while editing
		if (editing_[mi.base.bv]) {
			FontInfo font = mi.base.font;
			augmentFont(font, from_ascii("lyxtex"));
			Dimension namedim;
			mathed_string_dim(font, name(), namedim);
#if 0
			dim.wid += 2 + namedim.wid + 2 + 2;
			dim.asc = max(dim.asc, namedim.asc) + 2;
			dim.des = max(dim.des, namedim.des) + 2;
#endif
			dim.wid = max(1 + namedim.wid + 1, 2 + dim.wid + 2);
			dim.asc += 1 + namedim.height() + 1;
			dim.des += 2;
		}
	}

	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


int MathMacro::kerning(BufferView const * bv) const {
	if (displayMode_ == DISPLAY_NORMAL && !editing(bv))
		return expanded_.kerning(bv);
	else
		return 0;
}


void MathMacro::updateMacro(MacroContext const & mc) 
{
	if (validName()) {
		macro_ = mc.get(name());	    
		if (macro_ && macroBackup_ != *macro_) {
			macroBackup_ = *macro_;
			needsUpdate_ = true;
		}
	} else {
		macro_ = 0;
	}
}


void MathMacro::updateRepresentation(Cursor const * bvCur)
{
	// index of child where the cursor is (or -1 if none is edited)
	int curIdx = -1;
	if (bvCur) {
		curIdx = cursorIdx(*bvCur);
		previousCurIdx_[&bvCur->bv()] = curIdx;
	}

	// known macro?
	if (macro_ == 0)
		return;

	// update requires
	requires_ = macro_->requires();
	
	// non-normal mode? We are done!
	if (displayMode_ != DISPLAY_NORMAL)
		return;

	// macro changed?
	if (needsUpdate_) {
		needsUpdate_ = false;
		
		// get default values of macro
		vector<docstring> const & defaults = macro_->defaults();
		
		// create MathMacroArgumentValue objects pointing to the cells of the macro
		vector<MathData> values(nargs());
		for (size_t i = 0; i < nargs(); ++i) {
			ArgumentProxy * proxy;
			if (!cell(i).empty() 
			    || i >= defaults.size() 
			    || defaults[i].empty() 
			    || curIdx == (int)i)
				proxy = new ArgumentProxy(*this, i);
			else
				proxy = new ArgumentProxy(*this, i, defaults[i]);
			values[i].insert(0, MathAtom(proxy));
		}
		
		// expanding macro with the values
		macro_->expand(values, expanded_.cell(0));
	}		
}


void MathMacro::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);

	setPosCache(pi, x, y);
	int expx = x;
	int expy = y;

	if (displayMode_ == DISPLAY_INIT || displayMode_ == DISPLAY_INTERACTIVE_INIT) {		
		PainterInfo pi2(pi.base.bv, pi.pain);
		pi2.base.font.setColor(macro_ ? Color_latex : Color_error);
		//pi2.base.style = LM_ST_TEXT;
		pi2.pain.text(x, y, from_ascii("\\") + name(), pi2.base.font);
	} else if (displayMode_ == DISPLAY_UNFOLDED) {
		PainterInfo pi2(pi.base.bv, pi.pain);
		pi2.base.font.setColor(macro_ ? Color_latex : Color_error);
		//pi2.base.style = LM_ST_TEXT;
		pi2.pain.text(x, y, from_ascii("\\"), pi2.base.font);
		x += mathed_string_width(pi2.base.font, from_ascii("\\")) + 1;
		cell(0).draw(pi2, x, y);
		drawMarkers(pi2, expx, expy);
	} else {
		// warm up cells
		for (size_t i = 0; i < nargs(); ++i)
			cell(i).setXY(*pi.base.bv, x, y);

		if (editing_[pi.base.bv]) {
			// draw header and rectangle around
			FontInfo font = pi.base.font;
			augmentFont(font, from_ascii("lyxtex"));
			font.setSize(FONT_SIZE_TINY);
			font.setColor(Color_mathmacrolabel);
			Dimension namedim;
			mathed_string_dim(font, name(), namedim);
#if 0
			pi.pain.fillRectangle(x, y - dim.asc, 2 + namedim.width() + 2, dim.height(), Color_mathmacrobg);
			pi.pain.text(x + 2, y, name(), font);
			expx += 2 + namew + 2;
#endif
			pi.pain.fillRectangle(x, y - dim.asc, dim.wid, 1 + namedim.height() + 1, Color_mathmacrobg);
			pi.pain.text(x + 1, y - dim.asc + namedim.asc + 2, name(), font);
			expx += (dim.wid - expanded_.cell(0).dimension(*pi.base.bv).width()) / 2;

			pi.pain.enterMonochromeMode(Color_mathbg, Color_mathmacroblend);
			expanded_.cell(0).draw(pi, expx, expy);
			pi.pain.leaveMonochromeMode();
		} else
			expanded_.cell(0).draw(pi, expx, expy);

		// draw frame while editing
		if (editing_[pi.base.bv])
			pi.pain.rectangle(x, y - dim.asc, dim.wid, dim.height(), Color_mathmacroframe);
	}

	// another argument selected or edit mode changed?
	idx_type curIdx = cursorIdx(pi.base.bv->cursor());
	if (previousCurIdx_[pi.base.bv] != curIdx 
			|| editing_[pi.base.bv] != editMode(pi.base.bv->cursor()))
		pi.base.bv->cursor().updateFlags(Update::Force);
}


void MathMacro::drawSelection(PainterInfo & pi, int x, int y) const
{
	// We may have 0 arguments, but InsetMathNest requires at least one.
	if (cells_.size() > 0)
		InsetMathNest::drawSelection(pi, x, y);
}


void MathMacro::setDisplayMode(MathMacro::DisplayMode mode)
{
	if (displayMode_ != mode) {		
		// transfer name if changing from or to DISPLAY_UNFOLDED
		if (mode == DISPLAY_UNFOLDED) {
			cells_.resize(1);
			asArray(name_, cell(0));
		} else if (displayMode_ == DISPLAY_UNFOLDED) {
			name_ = asString(cell(0));
			cells_.resize(0);
		}

		displayMode_ = mode;
		needsUpdate_ = true;
	}
}


MathMacro::DisplayMode MathMacro::computeDisplayMode() const
{
	if (nextFoldMode_ == true && macro_ && !macro_->locked())
		return DISPLAY_NORMAL;
	else
		return DISPLAY_UNFOLDED;
}


bool MathMacro::validName() const
{
	docstring n = name();

	// empty name?
	if (n.size() == 0)
		return false;

	// converting back and force doesn't swallow anything?
	/*MathData ma;
	asArray(n, ma);
	if (asString(ma) != n)
		return false;*/

	// valid characters?
	for (size_t i = 0; i<n.size(); ++i) {
		if (!(n[i] >= 'a' && n[i] <= 'z') &&
				!(n[i] >= 'A' && n[i] <= 'Z')) 
			return false;
	}

	return true;
}


void MathMacro::validate(LaTeXFeatures & features) const
{
	if (!requires_.empty())
		features.require(requires_);

	if (name() == "binom" || name() == "mathcircumflex")
		features.require(to_utf8(name()));
}


void MathMacro::edit(Cursor & cur, bool left)
{
	cur.updateFlags(Update::Force);
	InsetMathNest::edit(cur, left);
}


Inset * MathMacro::editXY(Cursor & cur, int x, int y)
{
	// We may have 0 arguments, but InsetMathNest requires at least one.
	if (nargs() > 0) {
		cur.updateFlags(Update::Force);
		return InsetMathNest::editXY(cur, x, y);		
	} else
		return this;
}


void MathMacro::removeArgument(Inset::pos_type pos) {
	if (displayMode_ == DISPLAY_NORMAL) {
		BOOST_ASSERT(size_t(pos) < cells_.size());
		cells_.erase(cells_.begin() + pos);
		if (size_t(pos) < attachedArgsNum_)
			--attachedArgsNum_;
		if (size_t(pos) < optionals_) {
			--optionals_;
		}

		needsUpdate_ = true;
	}
}


void MathMacro::insertArgument(Inset::pos_type pos) {
	if (displayMode_ == DISPLAY_NORMAL) {
		BOOST_ASSERT(size_t(pos) <= cells_.size());
		cells_.insert(cells_.begin() + pos, MathData());
		if (size_t(pos) < attachedArgsNum_)
			++attachedArgsNum_;
		if (size_t(pos) < optionals_)
			++optionals_;

		needsUpdate_ = true;
	}
}


void MathMacro::detachArguments(vector<MathData> & args, bool strip)
{
	BOOST_ASSERT(displayMode_ == DISPLAY_NORMAL);	
	args = cells_;

	// strip off empty cells, but not more than arity-attachedArgsNum_
	if (strip) {
		size_t i;
		for (i = cells_.size(); i > attachedArgsNum_; --i)
			if (!cell(i - 1).empty()) break;
		args.resize(i);
	}

	attachedArgsNum_ = 0;
	expanded_.cell(0) = MathData();
	cells_.resize(0);

	needsUpdate_ = true;
}


void MathMacro::attachArguments(vector<MathData> const & args, size_t arity, int optionals)
{
	BOOST_ASSERT(displayMode_ == DISPLAY_NORMAL);
	cells_ = args;
	attachedArgsNum_ = args.size();
	cells_.resize(arity);
	expanded_.cell(0) = MathData();
	optionals_ = optionals;

	needsUpdate_ = true;
}


bool MathMacro::idxFirst(Cursor & cur) const 
{
	cur.updateFlags(Update::Force);
	return InsetMathNest::idxFirst(cur);
}


bool MathMacro::idxLast(Cursor & cur) const 
{
	cur.updateFlags(Update::Force);
	return InsetMathNest::idxLast(cur);
}


bool MathMacro::notifyCursorLeaves(Cursor & cur)
{
	cur.updateFlags(Update::Force);
	return InsetMathNest::notifyCursorLeaves(cur);
}


void MathMacro::fold(Cursor & cur)
{
	if (!nextFoldMode_) {
		nextFoldMode_ = true;
		cur.updateFlags(Update::Force);
	}
}


void MathMacro::unfold(Cursor & cur)
{
	if (nextFoldMode_) {
		nextFoldMode_ = false;
		cur.updateFlags(Update::Force);
	}
}


bool MathMacro::folded() const
{
	return nextFoldMode_;
}


void MathMacro::write(WriteStream & os) const
{
	// non-normal mode
	if (displayMode_ != DISPLAY_NORMAL) {
		os << "\\" << name() << " ";
		os.pendingSpace(true);
		return;
	}

	// normal mode
	BOOST_ASSERT(macro_);

	// optional arguments make macros fragile
	if (optionals_ > 0 && os.fragile())
		os << "\\protect";
	
	os << "\\" << name();
	bool first = true;
	
	// Optional arguments:
	// First find last non-empty optional argument
	idx_type emptyOptFrom = 0;
	idx_type i = 0;
	for (; i < cells_.size() && i < optionals_; ++i) {
		if (!cell(i).empty())
			emptyOptFrom = i + 1;
	}
	
	// print out optionals
	for (i=0; i < cells_.size() && i < emptyOptFrom; ++i) {
		first = false;
		os << "[" << cell(i) << "]";
	}
	
	// skip the tailing empty optionals
	i = optionals_;
	
	// Print remaining macros 
	for (; i < cells_.size(); ++i) {
		if (cell(i).size() == 1 
			 && cell(i)[0].nucleus()->asCharInset()) {
			if (first)
				os << " ";
			os << cell(i);
		} else
			os << "{" << cell(i) << "}";
		first = false;
	}

	// add space if there was no argument
	if (first)
		os.pendingSpace(true);
}


void MathMacro::maple(MapleStream & os) const
{
	lyx::maple(expanded_.cell(0), os);
}


void MathMacro::mathmlize(MathStream & os) const
{
	lyx::mathmlize(expanded_.cell(0), os);
}


void MathMacro::octave(OctaveStream & os) const
{
	lyx::octave(expanded_.cell(0), os);
}


void MathMacro::infoize(odocstream & os) const
{
	os << "Macro: " << name();
}


void MathMacro::infoize2(odocstream & os) const
{
	os << "Macro: " << name();

}


} // namespace lyx
