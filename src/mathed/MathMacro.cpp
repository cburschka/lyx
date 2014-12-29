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

#include "InsetMathChar.h"
#include "MathCompletionList.h"
#include "MathExtern.h"
#include "MathFactory.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "Buffer.h"
#include "BufferView.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "FuncStatus.h"
#include "FuncRequest.h"
#include "LaTeXFeatures.h"
#include "LyX.h"
#include "LyXRC.h"

#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include <ostream>
#include <vector>

using namespace lyx::support;
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
	InsetCode lyxCode() const { return ARGUMENT_PROXY_CODE; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const {
		mathMacro_.macro()->unlock();
		mathMacro_.cell(idx_).metrics(mi, dim);

		if (!mathMacro_.editMetrics(mi.base.bv)
		    && mathMacro_.cell(idx_).empty())
			def_.metrics(mi, dim);

		mathMacro_.macro()->lock();
	}
	// FIXME Other external things need similar treatment.
	///
	void mathmlize(MathStream & ms) const { ms << mathMacro_.cell(idx_); }
	///
	void htmlize(HtmlStream & ms) const { ms << mathMacro_.cell(idx_); }
	///
	void draw(PainterInfo & pi, int x, int y) const {
		if (mathMacro_.editMetrics(pi.base.bv)) {
			// The only way a ArgumentProxy can appear is in a cell of the
			// MathMacro. Moreover the cells are only drawn in the DISPLAY_FOLDED
			// mode and then, if the macro is edited the monochrome
			// mode is entered by the MathMacro before calling the cells' draw
			// method. Then eventually this code is reached and the proxy leaves
			// monochrome mode temporarely. Hence, if it is not in monochrome
			// here (and the assert triggers in pain.leaveMonochromeMode())
			// it's a bug.
			pi.pain.leaveMonochromeMode();
			mathMacro_.cell(idx_).draw(pi, x, y);
			pi.pain.enterMonochromeMode(Color_mathbg, Color_mathmacroblend);
		} else if (mathMacro_.cell(idx_).empty()) {
			mathMacro_.cell(idx_).setXY(*pi.base.bv, x, y);
			def_.draw(pi, x, y);
		} else
			mathMacro_.cell(idx_).draw(pi, x, y);
	}
	///
	size_t idx() const { return idx_; }
	///
	int kerning(BufferView const * bv) const
	{
		if (mathMacro_.editMetrics(bv)
		    || !mathMacro_.cell(idx_).empty())
			return mathMacro_.cell(idx_).kerning(bv);
		else
			return def_.kerning(bv);
	}

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


MathMacro::MathMacro(Buffer * buf, docstring const & name)
	: InsetMathNest(buf, 0), name_(name), displayMode_(DISPLAY_INIT),
		expanded_(buf), attachedArgsNum_(0), optionals_(0), nextFoldMode_(true),
		macroBackup_(buf), macro_(0), needsUpdate_(false),
		isUpdating_(false), appetite_(9)
{}


Inset * MathMacro::clone() const
{
	MathMacro * copy = new MathMacro(*this);
	copy->needsUpdate_ = true;
	//copy->expanded_.cell(0).clear();
	return copy;
}


void MathMacro::normalize(NormalStream & os) const
{
	os << "[macro " << name();
	for (size_t i = 0; i < nargs(); ++i)
		os << ' ' << cell(i);
	os << ']';
}


docstring MathMacro::name() const
{
	if (displayMode_ == DISPLAY_UNFOLDED)
		return asString(cell(0));

	return name_;
}


void MathMacro::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x,	int & y) const
{
	// We may have 0 arguments, but InsetMathNest requires at least one.
	if (nargs() > 0)
		InsetMathNest::cursorPos(bv, sl, boundary, x, y);
}


bool MathMacro::editMode(BufferView const * bv) const {
	// find this in cursor trace
	Cursor const & cur = bv->cursor();
	for (size_t i = 0; i != cur.depth(); ++i)
		if (&cur[i].inset() == this) {
			// look if there is no other macro in edit mode above
			++i;
			for (; i != cur.depth(); ++i) {
				InsetMath * im = cur[i].asInsetMath();
				if (im) {
					MathMacro const * macro = im->asMacro();
					if (macro && macro->displayMode() == DISPLAY_NORMAL)
						return false;
				}
			}

			// ok, none found, I am the highest one
			return true;
		}

	return false;
}


bool MathMacro::editMetrics(BufferView const * bv) const
{
	return editing_[bv];
}


void MathMacro::metrics(MetricsInfo & mi, Dimension & dim) const
{
	// set edit mode for which we will have calculated metrics. But only
	editing_[mi.base.bv] = editMode(mi.base.bv);

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
	} else if (lyxrc.macro_edit_style == LyXRC::MACRO_EDIT_LIST
		   && editing_[mi.base.bv]) {
		// Macro will be edited in a old-style list mode here:

		LBUFERR(macro_);
		Dimension fontDim;
		FontInfo labelFont = sane_font;
		math_font_max_dim(labelFont, fontDim.asc, fontDim.des);

		// get dimension of components of list view
		Dimension nameDim;
		nameDim.wid = mathed_string_width(mi.base.font, from_ascii("Macro \\") + name() + ": ");
		nameDim.asc = fontDim.asc;
		nameDim.des = fontDim.des;

		Dimension argDim;
		argDim.wid = mathed_string_width(labelFont, from_ascii("#9: "));
		argDim.asc = fontDim.asc;
		argDim.des = fontDim.des;

		Dimension defDim;
		definition_.metrics(mi, defDim);

		// add them up
		dim.wid = nameDim.wid + defDim.wid;
		dim.asc = max(nameDim.asc, defDim.asc);
		dim.des = max(nameDim.des, defDim.des);

		for (idx_type i = 0; i < nargs(); ++i) {
			Dimension cdim;
			cell(i).metrics(mi, cdim);
			dim.des += max(argDim.height(), cdim.height()) + 1;
			dim.wid = max(dim.wid, argDim.wid + cdim.wid);
		}

		// make space for box and markers, 2 pixels
		dim.asc += 1;
		dim.des += 1;
		dim.wid += 2;
		metricsMarkers2(dim);
	} else {
		LBUFERR(macro_);

		// calculate metrics, hoping that all cells are seen
		macro_->lock();
		expanded_.cell(0).metrics(mi, dim);

		// otherwise do a manual metrics call
		CoordCache & coords = mi.base.bv->coordCache();
		for (idx_type i = 0; i < nargs(); ++i) {
			if (!coords.getArrays().hasDim(&cell(i))) {
				Dimension tdim;
				cell(i).metrics(mi, tdim);
			}
		}
		macro_->unlock();

		// calculate dimension with label while editing
		if (lyxrc.macro_edit_style == LyXRC::MACRO_EDIT_INLINE_BOX
		    && editing_[mi.base.bv]) {
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
}


int MathMacro::kerning(BufferView const * bv) const {
	if (displayMode_ == DISPLAY_NORMAL && !editing_[bv])
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


class MathMacro::UpdateLocker
{
public:
	explicit UpdateLocker(MathMacro & mm) : mac(mm)
	{
		mac.isUpdating_ = true;
	}
	~UpdateLocker() { mac.isUpdating_ = false; }
private:
	MathMacro & mac;
};
/** Avoid wrong usage of UpdateLocker.
    To avoid wrong usage:
    UpdateLocker(...); // wrong
    UpdateLocker locker(...); // right
*/
#define UpdateLocker(x) unnamed_UpdateLocker;
// Tip gotten from Bobby Schmidt's column in C/C++ Users Journal


void MathMacro::updateRepresentation(Cursor * cur, MacroContext const & mc,
		UpdateType utype)
{
	// block recursive calls (bug 8999)
	if (isUpdating_)
		return;

	UpdateLocker locker(*this);

	// known macro?
	if (macro_ == 0)
		return;

	// update requires
	requires_ = macro_->requires();

	if (!needsUpdate_
		// non-normal mode? We are done!
		|| (displayMode_ != DISPLAY_NORMAL))
		return;

	needsUpdate_ = false;

	// get default values of macro
	vector<docstring> const & defaults = macro_->defaults();

	// create MathMacroArgumentValue objects pointing to the cells of the macro
	vector<MathData> values(nargs());
	for (size_t i = 0; i < nargs(); ++i) {
		ArgumentProxy * proxy;
		if (i < defaults.size())
			proxy = new ArgumentProxy(*this, i, defaults[i]);
		else
			proxy = new ArgumentProxy(*this, i);
		values[i].insert(0, MathAtom(proxy));
	}
	// expanding macro with the values
	// Only update the argument macros if anything was expanded, otherwise
	// we would get an endless loop (bug 9140). UpdateLocker does not work
	// in this case, since MacroData::expand() creates new MathMacro
	// objects, so this would be a different recursion path than the one
	// protected by UpdateLocker.
	if (macro_->expand(values, expanded_.cell(0))) {
		if (utype == OutputUpdate && !expanded_.cell(0).empty())
			expanded_.cell(0).updateMacros(cur, mc, utype);
	}
	// get definition for list edit mode
	docstring const & display = macro_->display();
	asArray(display.empty() ? macro_->definition() : display, definition_);
}


void MathMacro::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);

	setPosCache(pi, x, y);
	int expx = x;
	int expy = y;

	if (displayMode_ == DISPLAY_INIT || displayMode_ == DISPLAY_INTERACTIVE_INIT) {
		FontSetChanger dummy(pi.base, "lyxtex");
		pi.pain.text(x, y, from_ascii("\\") + name(), pi.base.font);
	} else if (displayMode_ == DISPLAY_UNFOLDED) {
		FontSetChanger dummy(pi.base, "lyxtex");
		pi.pain.text(x, y, from_ascii("\\"), pi.base.font);
		x += mathed_string_width(pi.base.font, from_ascii("\\")) + 1;
		cell(0).draw(pi, x, y);
		drawMarkers(pi, expx, expy);
	} else if (lyxrc.macro_edit_style == LyXRC::MACRO_EDIT_LIST
		   && editing_[pi.base.bv]) {
		// Macro will be edited in a old-style list mode here:

		CoordCache const & coords = pi.base.bv->coordCache();
		FontInfo const & labelFont = sane_font;

		// markers and box needs two pixels
		x += 2;

		// get maximal font height
		Dimension fontDim;
		math_font_max_dim(pi.base.font, fontDim.asc, fontDim.des);

		// draw label
		docstring label = from_ascii("Macro \\") + name() + from_ascii(": ");
		pi.pain.text(x, y, label, labelFont);
		x += mathed_string_width(labelFont, label);

		// draw definition
		definition_.draw(pi, x, y);
		Dimension const & defDim = coords.getArrays().dim(&definition_);
		y += max(fontDim.des, defDim.des);

		// draw parameters
		docstring str = from_ascii("#9");
		int strw1 = mathed_string_width(labelFont, from_ascii("#9"));
		int strw2 = mathed_string_width(labelFont, from_ascii(": "));

		for (idx_type i = 0; i < nargs(); ++i) {
			// position of label
			Dimension const & cdim = coords.getArrays().dim(&cell(i));
			x = expx + 2;
			y += max(fontDim.asc, cdim.asc) + 1;

			// draw label
			str[1] = '1' + i;
			pi.pain.text(x, y, str, labelFont);
			x += strw1;
			pi.pain.text(x, y, from_ascii(":"), labelFont);
			x += strw2;

			// draw paramter
			cell(i).draw(pi, x, y);

			// next line
			y += max(fontDim.des, cdim.des);
		}

		pi.pain.rectangle(expx + 1, expy - dim.asc + 1, dim.wid - 3,
				  dim.height() - 2, Color_mathmacroframe);
		drawMarkers2(pi, expx, expy);
	} else {
		bool drawBox = lyxrc.macro_edit_style == LyXRC::MACRO_EDIT_INLINE_BOX;

		// warm up cells
		for (size_t i = 0; i < nargs(); ++i)
			cell(i).setXY(*pi.base.bv, x, y);

		if (drawBox && editing_[pi.base.bv]) {
			// draw header and rectangle around
			FontInfo font = pi.base.font;
			augmentFont(font, from_ascii("lyxtex"));
			font.setSize(FONT_SIZE_TINY);
			font.setColor(Color_mathmacrolabel);
			Dimension namedim;
			mathed_string_dim(font, name(), namedim);

			pi.pain.fillRectangle(x, y - dim.asc, dim.wid, 1 + namedim.height() + 1, Color_mathmacrobg);
			pi.pain.text(x + 1, y - dim.asc + namedim.asc + 2, name(), font);
			expx += (dim.wid - expanded_.cell(0).dimension(*pi.base.bv).width()) / 2;
		}

		if (editing_[pi.base.bv]) {
			pi.pain.enterMonochromeMode(Color_mathbg, Color_mathmacroblend);
			expanded_.cell(0).draw(pi, expx, expy);
			pi.pain.leaveMonochromeMode();

			if (drawBox)
				pi.pain.rectangle(x, y - dim.asc, dim.wid,
						  dim.height(), Color_mathmacroframe);
		} else
			expanded_.cell(0).draw(pi, expx, expy);

		if (!drawBox)
			drawMarkers(pi, x, y);
	}

	// edit mode changed?
	if (editing_[pi.base.bv] != editMode(pi.base.bv))
		pi.base.bv->cursor().screenUpdateFlags(Update::SinglePar);
}


void MathMacro::drawSelection(PainterInfo & pi, int x, int y) const
{
	// We may have 0 arguments, but InsetMathNest requires at least one.
	if (!cells_.empty())
		InsetMathNest::drawSelection(pi, x, y);
}


void MathMacro::setDisplayMode(MathMacro::DisplayMode mode, int appetite)
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

	// the interactive init mode is non-greedy by default
	if (appetite == -1)
		appetite_ = (mode == DISPLAY_INTERACTIVE_INIT) ? 0 : 9;
	else
		appetite_ = size_t(appetite);
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

	if (n.empty())
		return false;

	// converting back and force doesn't swallow anything?
	/*MathData ma;
	asArray(n, ma);
	if (asString(ma) != n)
		return false;*/

	// valid characters?
	for (size_t i = 0; i<n.size(); ++i) {
		if (!(n[i] >= 'a' && n[i] <= 'z')
		    && !(n[i] >= 'A' && n[i] <= 'Z')
		    && n[i] != '*')
			return false;
	}

	return true;
}


void MathMacro::validate(LaTeXFeatures & features) const
{
	if (!requires_.empty())
		features.require(requires_);

	if (name() == "binom")
		features.require("binom");

	// validate the cells and the definition
	if (displayMode() == DISPLAY_NORMAL) {
		definition_.validate(features);
		InsetMathNest::validate(features);
	}
}


void MathMacro::edit(Cursor & cur, bool front, EntryDirection entry_from)
{
	cur.screenUpdateFlags(Update::SinglePar);
	InsetMathNest::edit(cur, front, entry_from);
}


Inset * MathMacro::editXY(Cursor & cur, int x, int y)
{
	// We may have 0 arguments, but InsetMathNest requires at least one.
	if (nargs() > 0) {
		cur.screenUpdateFlags(Update::SinglePar);
		return InsetMathNest::editXY(cur, x, y);
	} else
		return this;
}


void MathMacro::removeArgument(Inset::pos_type pos) {
	if (displayMode_ == DISPLAY_NORMAL) {
		LASSERT(size_t(pos) < cells_.size(), return);
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
		LASSERT(size_t(pos) <= cells_.size(), return);
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
	LASSERT(displayMode_ == DISPLAY_NORMAL, return);
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
	LASSERT(displayMode_ == DISPLAY_NORMAL, return);
	cells_ = args;
	attachedArgsNum_ = args.size();
	cells_.resize(arity);
	expanded_.cell(0) = MathData();
	optionals_ = optionals;

	needsUpdate_ = true;
}


bool MathMacro::idxFirst(Cursor & cur) const
{
	cur.screenUpdateFlags(Update::SinglePar);
	return InsetMathNest::idxFirst(cur);
}


bool MathMacro::idxLast(Cursor & cur) const
{
	cur.screenUpdateFlags(Update::SinglePar);
	return InsetMathNest::idxLast(cur);
}


bool MathMacro::notifyCursorLeaves(Cursor const & old, Cursor & cur)
{
	if (displayMode_ == DISPLAY_UNFOLDED) {
		docstring const & unfolded_name = name();
		if (unfolded_name != name_) {
			// The macro name was changed
			Cursor inset_cursor = old;
			int macroSlice = inset_cursor.find(this);
			// returning true means the cursor is "now" invalid,
			// which it was.
			LASSERT(macroSlice != -1, return true);
			inset_cursor.cutOff(macroSlice);
			inset_cursor.recordUndoInset();
			inset_cursor.pop();
			inset_cursor.cell().erase(inset_cursor.pos());
			inset_cursor.cell().insert(inset_cursor.pos(),
				createInsetMath(unfolded_name, cur.buffer()));
			cur.resetAnchor();
			cur.screenUpdateFlags(cur.result().screenUpdate() | Update::SinglePar);
			return true;
		}
	}
	cur.screenUpdateFlags(Update::Force);
	return InsetMathNest::notifyCursorLeaves(old, cur);
}


void MathMacro::fold(Cursor & cur)
{
	if (!nextFoldMode_) {
		nextFoldMode_ = true;
		cur.screenUpdateFlags(Update::SinglePar);
	}
}


void MathMacro::unfold(Cursor & cur)
{
	if (nextFoldMode_) {
		nextFoldMode_ = false;
		cur.screenUpdateFlags(Update::SinglePar);
	}
}


bool MathMacro::folded() const
{
	return nextFoldMode_;
}


void MathMacro::write(WriteStream & os) const
{
	MathEnsurer ensurer(os, macro_ != 0, true);

	// non-normal mode
	if (displayMode_ != DISPLAY_NORMAL) {
		os << "\\" << name();
		if (name().size() != 1 || isAlphaASCII(name()[0]))
			os.pendingSpace(true);
		return;
	}

	// normal mode
	// we should be ok to continue even if this fails.
	LATTEST(macro_);

	// Always protect macros in a fragile environment
	if (os.fragile())
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

	// Print remaining arguments
	for (; i < cells_.size(); ++i) {
		if (cell(i).size() == 1
			&& cell(i)[0].nucleus()->asCharInset()
			&& isASCII(cell(i)[0].nucleus()->asCharInset()->getChar())) {
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
	MathWordList const & words = mathedWordList();
	MathWordList::const_iterator it = words.find(name());
	if (it != words.end()) {
		docstring const xmlname = it->second.xmlname;
		if (!xmlname.empty()) {
			char const * type = it->second.MathMLtype();
			os << '<' << type << "> " << xmlname << " /<"
			   << type << '>';
			return;
		}
	}
	MathData const & data = expanded_.cell(0);
	if (data.empty()) {
		// this means that we do not recognize the macro
		throw MathExportException();
	}
	os << data;
}


void MathMacro::htmlize(HtmlStream & os) const
{
	MathWordList const & words = mathedWordList();
	MathWordList::const_iterator it = words.find(name());
	if (it != words.end()) {
		docstring const xmlname = it->second.xmlname;
		if (!xmlname.empty()) {
			os << ' ' << xmlname << ' ';
			return;
		}
	}
	MathData const & data = expanded_.cell(0);
	if (data.empty()) {
		// this means that we do not recognize the macro
		throw MathExportException();
	}
	os << data;
}


void MathMacro::octave(OctaveStream & os) const
{
	lyx::octave(expanded_.cell(0), os);
}


void MathMacro::infoize(odocstream & os) const
{
	os << bformat(_("Macro: %1$s"), name());
}


void MathMacro::infoize2(odocstream & os) const
{
	os << bformat(_("Macro: %1$s"), name());
}


bool MathMacro::completionSupported(Cursor const & cur) const
{
	if (displayMode() != DISPLAY_UNFOLDED)
		return InsetMathNest::completionSupported(cur);

	return lyxrc.completion_popup_math
		&& displayMode() == DISPLAY_UNFOLDED
		&& cur.bv().cursor().pos() == int(name().size());
}


bool MathMacro::inlineCompletionSupported(Cursor const & cur) const
{
	if (displayMode() != DISPLAY_UNFOLDED)
		return InsetMathNest::inlineCompletionSupported(cur);

	return lyxrc.completion_inline_math
		&& displayMode() == DISPLAY_UNFOLDED
		&& cur.bv().cursor().pos() == int(name().size());
}


bool MathMacro::automaticInlineCompletion() const
{
	if (displayMode() != DISPLAY_UNFOLDED)
		return InsetMathNest::automaticInlineCompletion();

	return lyxrc.completion_inline_math;
}


bool MathMacro::automaticPopupCompletion() const
{
	if (displayMode() != DISPLAY_UNFOLDED)
		return InsetMathNest::automaticPopupCompletion();

	return lyxrc.completion_popup_math;
}


CompletionList const *
MathMacro::createCompletionList(Cursor const & cur) const
{
	if (displayMode() != DISPLAY_UNFOLDED)
		return InsetMathNest::createCompletionList(cur);

	return new MathCompletionList(cur.bv().cursor());
}


docstring MathMacro::completionPrefix(Cursor const & cur) const
{
	if (displayMode() != DISPLAY_UNFOLDED)
		return InsetMathNest::completionPrefix(cur);

	if (!completionSupported(cur))
		return docstring();

	return "\\" + name();
}


bool MathMacro::insertCompletion(Cursor & cur, docstring const & s,
					bool finished)
{
	if (displayMode() != DISPLAY_UNFOLDED)
		return InsetMathNest::insertCompletion(cur, s, finished);

	if (!completionSupported(cur))
		return false;

	// append completion
	docstring newName = name() + s;
	asArray(newName, cell(0));
	cur.bv().cursor().pos() = name().size();
	cur.screenUpdateFlags(Update::SinglePar);

	// finish macro
	if (finished) {
		cur.bv().cursor().pop();
		++cur.bv().cursor().pos();
		cur.screenUpdateFlags(Update::SinglePar);
	}

	return true;
}


void MathMacro::completionPosAndDim(Cursor const & cur, int & x, int & y,
	Dimension & dim) const
{
	if (displayMode() != DISPLAY_UNFOLDED)
		InsetMathNest::completionPosAndDim(cur, x, y, dim);

	// get inset dimensions
	dim = cur.bv().coordCache().insets().dim(this);
	// FIXME: these 3 are no accurate, but should depend on the font.
	// Now the popup jumps down if you enter a char with descent > 0.
	dim.des += 3;
	dim.asc += 3;

	// and position
	Point xy
	= cur.bv().coordCache().insets().xy(this);
	x = xy.x_;
	y = xy.y_;
}


} // namespace lyx
