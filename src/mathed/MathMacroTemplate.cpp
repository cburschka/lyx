/**
 * \file MathMacroTemplate.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MathMacroTemplate.h"

#include "InsetMathBrace.h"
#include "InsetMathChar.h"
#include "InsetMathHull.h"
#include "InsetMathSqrt.h"
#include "MathMacro.h"
#include "MathMacroArgument.h"
#include "MathStream.h"
#include "MathParser.h"
#include "MathSupport.h"
#include "MathMacroArgument.h"

#include "Buffer.h"
#include "BufferView.h"
#include "Color.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "DocIterator.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "TocBackend.h"

#include "frontends/Painter.h"

#include "insets/RenderPreview.h"

#include "support/lassert.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/gettext.h"
#include "support/docstream.h"
#include "support/lstrings.h"

#include <set>
#include <sstream>

using namespace std;

namespace lyx {

using support::bformat;

//////////////////////////////////////////////////////////////////////

class InsetLabelBox : public InsetMathNest {
public:
	///
	InsetLabelBox(Buffer * buf, MathAtom const & atom, docstring label,
		      MathMacroTemplate const & parent, bool frame = false);
	InsetLabelBox(Buffer * buf, docstring label, MathMacroTemplate const & parent,
		      bool frame = false);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;

protected:
	///
	MathMacroTemplate const & parent_;
	///
	Inset * clone() const;
	///
	docstring const label_;
	///
	bool frame_;
};


InsetLabelBox::InsetLabelBox(Buffer * buf, MathAtom const & atom, docstring label,
	MathMacroTemplate const & parent, bool frame)
	: InsetMathNest(buf, 1), parent_(parent), label_(label), frame_(frame)
{
	cell(0).insert(0, atom);
}


InsetLabelBox::InsetLabelBox(Buffer * buf, docstring label,
			     MathMacroTemplate const & parent, bool frame)
	: InsetMathNest(buf, 1), parent_(parent), label_(label), frame_(frame)
{
}


Inset * InsetLabelBox::clone() const
{
	return new InsetLabelBox(*this);
}


void InsetLabelBox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	// kernel
	cell(0).metrics(mi, dim);

	// frame
	if (frame_) {
		dim.wid += 6;
		dim.asc += 5;
		dim.des += 5;
	}

	// adjust to common height in main metrics phase
	if (!parent_.premetrics()) {
		dim.asc = max(dim.asc, parent_.commonLabelBoxAscent());
		dim.des = max(dim.des, parent_.commonLabelBoxDescent());
	}

	// label
	if (parent_.editing(mi.base.bv) && label_.length() > 0) {
		// grey
		FontInfo font = sane_font;
		font.setSize(FONT_SIZE_TINY);
		font.setColor(Color_mathmacrolabel);

		// make space for label and box
		int lwid = mathed_string_width(font, label_);
		int maxasc;
		int maxdes;
		math_font_max_dim(font, maxasc, maxdes);

		dim.wid = max(dim.wid, lwid + 2);

		// space for the label
		if (!parent_.premetrics())
			dim.des += maxasc + maxdes + 1;
	}
}


void InsetLabelBox::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	Dimension const cdim = cell(0).dimension(*pi.base.bv);

	// kernel
	cell(0).draw(pi, x + (dim.wid - cdim.wid) / 2, y);

	// label
	if (parent_.editing(pi.base.bv) && label_.length() > 0) {
		// grey
		FontInfo font = sane_font;
		font.setSize(FONT_SIZE_TINY);
		font.setColor(Color_mathmacrolabel);

		// make space for label and box
		int lwid = mathed_string_width(font, label_);
		int maxasc;
		int maxdes;
		math_font_max_dim(font, maxasc, maxdes);

		if (lwid < dim.wid)
			pi.pain.text(x + (dim.wid - lwid) / 2, y + dim.des - maxdes, label_, font);
		else
			pi.pain.text(x, y + dim.des - maxdes, label_, font);
	}

	// draw frame
	int boxHeight = parent_.commonLabelBoxAscent() + parent_.commonLabelBoxDescent();
	if (frame_) {
		pi.pain.rectangle(x + 1, y - dim.ascent() + 1,
				  dim.wid - 2, boxHeight - 2,
				  Color_mathline);
	}
}


//////////////////////////////////////////////////////////////////////

class DisplayLabelBox : public InsetLabelBox {
public:
	///
	DisplayLabelBox(Buffer * buf, MathAtom const & atom, docstring label,
			MathMacroTemplate const & parent);

	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;

protected:
	///
	Inset * clone() const;
};


DisplayLabelBox::DisplayLabelBox(Buffer * buf, MathAtom const & atom,
				 docstring label,
				 MathMacroTemplate const & parent)
	: InsetLabelBox(buf, atom, label, parent, true)
{
}



Inset * DisplayLabelBox::clone() const
{
	return new DisplayLabelBox(*this);
}


void DisplayLabelBox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetLabelBox::metrics(mi, dim);
	if (!parent_.editing(mi.base.bv)
	    && parent_.cell(parent_.displayIdx()).empty()) {
		dim.wid = 0;
		dim.asc = 0;
		dim.des = 0;
	}
}


void DisplayLabelBox::draw(PainterInfo & pi, int x, int y) const
{
	if (parent_.editing(pi.base.bv)
	    || !parent_.cell(parent_.displayIdx()).empty()) {
		InsetLabelBox::draw(pi, x, y);
	} else {
		bool enabled = pi.pain.isDrawingEnabled();
		pi.pain.setDrawingEnabled(false);
		InsetLabelBox::draw(pi, x, y);
		pi.pain.setDrawingEnabled(enabled);
	}
}


//////////////////////////////////////////////////////////////////////

class InsetMathWrapper : public InsetMath {
public:
	///
	InsetMathWrapper(MathData const * value) : value_(value) {}
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;

private:
	///
	Inset * clone() const;
	///
	MathData const * value_;
};


Inset * InsetMathWrapper::clone() const
{
	return new InsetMathWrapper(*this);
}


void InsetMathWrapper::metrics(MetricsInfo & mi, Dimension & dim) const
{
	value_->metrics(mi, dim);
	//metricsMarkers2(dim);
}


void InsetMathWrapper::draw(PainterInfo & pi, int x, int y) const
{
	value_->draw(pi, x, y);
	//drawMarkers(pi, x, y);
}


///////////////////////////////////////////////////////////////////////
class InsetColoredCell : public InsetMathNest {
public:
	///
	InsetColoredCell(Buffer * buf, ColorCode min, ColorCode max);
	///
	InsetColoredCell(Buffer * buf, ColorCode min, ColorCode max, MathAtom const & atom);
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;

protected:
	///
	Inset * clone() const;
	///
	ColorCode min_;
	///
	ColorCode max_;
};


InsetColoredCell::InsetColoredCell(Buffer * buf, ColorCode min, ColorCode max)
	: InsetMathNest(buf, 1), min_(min), max_(max)
{
}


InsetColoredCell::InsetColoredCell(Buffer * buf, ColorCode min, ColorCode max, MathAtom const & atom)
	: InsetMathNest(buf, 1), min_(min), max_(max)
{
	cell(0).insert(0, atom);
}


Inset * InsetColoredCell::clone() const
{
	return new InsetColoredCell(*this);
}


void InsetColoredCell::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
}


void InsetColoredCell::draw(PainterInfo & pi, int x, int y) const
{
	pi.pain.enterMonochromeMode(min_, max_);
	cell(0).draw(pi, x, y);
	pi.pain.leaveMonochromeMode();
}


///////////////////////////////////////////////////////////////////////

class InsetNameWrapper : public InsetMathWrapper {
public:
	///
	InsetNameWrapper(MathData const * value, MathMacroTemplate const & parent);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;

private:
	///
	MathMacroTemplate const & parent_;
	///
	Inset * clone() const;
};


InsetNameWrapper::InsetNameWrapper(MathData const * value,
				   MathMacroTemplate const & parent)
	: InsetMathWrapper(value), parent_(parent)
{
}


Inset * InsetNameWrapper::clone() const
{
	return new InsetNameWrapper(*this);
}


void InsetNameWrapper::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetMathWrapper::metrics(mi, dim);
	dim.wid += mathed_string_width(mi.base.font, from_ascii("\\"));
}


void InsetNameWrapper::draw(PainterInfo & pi, int x, int y) const
{
	// create fonts
	PainterInfo namepi = pi;
	if (parent_.validMacro())
		namepi.base.font.setColor(Color_latex);
	else
		namepi.base.font.setColor(Color_error);

	// draw backslash
	pi.pain.text(x, y, from_ascii("\\"), namepi.base.font);
	x += mathed_string_width(namepi.base.font, from_ascii("\\"));

	// draw name
	InsetMathWrapper::draw(namepi, x, y);
}


///////////////////////////////////////////////////////////////////////


MathMacroTemplate::MathMacroTemplate(Buffer * buf)
	: InsetMathNest(buf, 3), numargs_(0), argsInLook_(0), optionals_(0),
	  type_(MacroTypeNewcommand), redefinition_(false), lookOutdated_(true),
	  premetrics_(false), labelBoxAscent_(0), labelBoxDescent_(0)
{
	initMath();
}


MathMacroTemplate::MathMacroTemplate(Buffer * buf, docstring const & name, int numargs,
	int optionals, MacroType type, vector<MathData> const & optionalValues,
	MathData const & def, MathData const & display)
	: InsetMathNest(buf, optionals + 3), numargs_(numargs), argsInLook_(numargs),
	  optionals_(optionals), optionalValues_(optionalValues),
	  type_(type), redefinition_(false), lookOutdated_(true),
	  premetrics_(false), labelBoxAscent_(0), labelBoxDescent_(0)
{
	initMath();

	if (numargs_ > 9)
		lyxerr << "MathMacroTemplate::MathMacroTemplate: wrong # of arguments: "
			<< numargs_ << endl;

	asArray(name, cell(0));
	optionalValues_.resize(9);
	for (int i = 0; i < optionals_; ++i)
		cell(optIdx(i)) = optionalValues_[i];
	cell(defIdx()) = def;
	cell(displayIdx()) = display;

	updateLook();
}


bool MathMacroTemplate::fromString(docstring const & str)
{
	MathData ar(buffer_);
	mathed_parse_cell(ar, str, Parse::NORMAL);
	if (ar.size() != 1 || !ar[0]->asMacroTemplate()) {
		lyxerr << "Cannot read macro from '" << ar << "'" << endl;
		asArray(from_ascii("invalidmacro"), cell(0));
		// The macro template does not make sense after this.
		return false;
	}
	operator=( *(ar[0]->asMacroTemplate()) );

	updateLook();
	return true;
}


Inset * MathMacroTemplate::clone() const
{
	MathMacroTemplate * inset = new MathMacroTemplate(*this);
	// the parent pointers of the proxy insets above will point to
	// to the old template. Hence, the look must be updated.
	inset->updateLook();
	return inset;
}


docstring MathMacroTemplate::name() const
{
	return asString(cell(0));
}


void MathMacroTemplate::updateToContext(MacroContext const & mc)
{
	redefinition_ = mc.get(name()) != 0;
}


void MathMacroTemplate::updateLook() const
{
	lookOutdated_ = true;
}


void MathMacroTemplate::createLook(int args) const
{
	look_.clear();
	argsInLook_ = args;

	// \foo
	look_.push_back(MathAtom(
		new InsetLabelBox(buffer_, _("Name"), *this, false)));
	MathData & nameData = look_[look_.size() - 1].nucleus()->cell(0);
	nameData.push_back(MathAtom(new InsetNameWrapper(&cell(0), *this)));

	// [#1][#2]
	int i = 0;
	if (optionals_ > 0) {
		look_.push_back(MathAtom(
			new InsetLabelBox(buffer_, _("optional"), *this, false)));
		
		MathData * optData = &look_[look_.size() - 1].nucleus()->cell(0);
		for (; i < optionals_; ++i) {
			// color it light grey, if it is to be removed when the cursor leaves
			if (i == argsInLook_) {
				optData->push_back(MathAtom(
					new InsetColoredCell(buffer_, Color_mathbg, Color_mathmacrooldarg)));
				optData = &(*optData)[optData->size() - 1].nucleus()->cell(0);
			}

			optData->push_back(MathAtom(new InsetMathChar('[')));
			optData->push_back(MathAtom(new InsetMathWrapper(&cell(1 + i))));
			optData->push_back(MathAtom(new InsetMathChar(']')));
		}
	}

	// {#3}{#4}
	for (; i < numargs_; ++i) {
		MathData arg;
		arg.push_back(MathAtom(new MathMacroArgument(i + 1)));
		if (i >= argsInLook_) {
			look_.push_back(MathAtom(new InsetColoredCell(buffer_,
				Color_mathbg, Color_mathmacrooldarg,
				MathAtom(new InsetMathBrace(arg)))));
		} else
			look_.push_back(MathAtom(new InsetMathBrace(arg)));
	}
	for (; i < argsInLook_; ++i) {
		MathData arg;
		arg.push_back(MathAtom(new MathMacroArgument(i + 1)));
		look_.push_back(MathAtom(new InsetColoredCell(buffer_,
			Color_mathbg, Color_mathmacronewarg,
			MathAtom(new InsetMathBrace(arg)))));
	}
	
	// :=
	look_.push_back(MathAtom(new InsetMathChar(':')));
	look_.push_back(MathAtom(new InsetMathChar('=')));

	// definition
	look_.push_back(MathAtom(
		new InsetLabelBox(buffer_, MathAtom(
			new InsetMathWrapper(&cell(defIdx()))), _("TeX"), *this,	true)));

	// display
	look_.push_back(MathAtom(
		new DisplayLabelBox(buffer_, MathAtom(
			new InsetMathWrapper(&cell(displayIdx()))), _("LyX"), *this)));
}


void MathMacroTemplate::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Changer dummy1 = mi.base.changeFontSet("mathnormal");
	Changer dummy2 = mi.base.changeStyle(LM_ST_TEXT);

	// valid macro?
	MacroData const * macro = 0;
	if (validName())
		macro = mi.macrocontext.get(name());

	// update look?
	int argsInDef = maxArgumentInDefinition();
	if (lookOutdated_ || argsInDef != argsInLook_) {
		lookOutdated_ = false;
		createLook(argsInDef);
	}

	/// metrics for inset contents
	if (macro)
		macro->lock();

	// first phase, premetric:
	premetrics_ = true;
	look_.metrics(mi, dim);
	labelBoxAscent_ = dim.asc;
	labelBoxDescent_ = dim.des;

	// second phase, main metric:
	premetrics_ = false;
	look_.metrics(mi, dim);

	if (macro)
		macro->unlock();

	dim.wid += 6;
	dim.des += 2;
	dim.asc += 2;

	setDimCache(mi, dim);
}


void MathMacroTemplate::draw(PainterInfo & pi, int x, int y) const
{
	// FIXME: Calling Changer on the same object repeatedly is inefficient.
	Changer dummy0 = pi.base.font.changeColor(Color_math);
	Changer dummy1 = pi.base.changeFontSet("mathnormal");
	Changer dummy2 = pi.base.changeStyle(LM_ST_TEXT);

	setPosCache(pi, x, y);
	Dimension const dim = dimension(*pi.base.bv);

	// draw outer frame
	int const a = y - dim.asc + 1;
	int const w = dim.wid - 2;
	int const h = dim.height() - 2;
	pi.pain.rectangle(x, a, w, h, Color_mathframe);

	// just to be sure: set some dummy values for coord cache
	for (idx_type i = 0; i < nargs(); ++i)
		cell(i).setXY(*pi.base.bv, x, y);

	// draw contents
	look_.draw(pi, x + 3, y);
}


void MathMacroTemplate::edit(Cursor & cur, bool front, EntryDirection entry_from)
{
	updateLook();
	cur.screenUpdateFlags(Update::SinglePar);
	InsetMathNest::edit(cur, front, entry_from);
}


bool MathMacroTemplate::notifyCursorLeaves(Cursor const & old, Cursor & cur)
{
	unsigned int const nargs_before = nargs();
	commitEditChanges(cur, old);
	updateLook();
	cur.screenUpdateFlags(Update::Force);
	// If we have removed a cell, we might have invalidated the cursor
	return InsetMathNest::notifyCursorLeaves(old, cur)
		|| nargs() < nargs_before;
}


void MathMacroTemplate::removeArguments(Cursor & cur,
	DocIterator const & /*inset_pos*/, int from, int to)
{
	DocIterator it = doc_iterator_begin(&buffer(), this);
	for (; it; it.forwardChar()) {
		if (!it.nextInset())
			continue;
		if (it.nextInset()->lyxCode() != MATH_MACROARG_CODE)
			continue;
		MathMacroArgument * arg = static_cast<MathMacroArgument*>(it.nextInset());
		int n = arg->number() - 1;
		if (from <= n && n <= to) {
			int cellSlice = cur.find(it.cell());
			if (cellSlice != -1 && cur[cellSlice].pos() > it.pos())
				--cur[cellSlice].pos();

			it.cell().erase(it.pos());
		}
	}

	updateLook();
}


void MathMacroTemplate::shiftArguments(size_t from, int by)
{
	for (DocIterator it = doc_iterator_begin(&buffer(), this); it; it.forwardChar()) {
		if (!it.nextInset())
			continue;
		if (it.nextInset()->lyxCode() != MATH_MACROARG_CODE)
			continue;
		MathMacroArgument * arg = static_cast<MathMacroArgument*>(it.nextInset());
		if (arg->number() >= int(from) + 1)
			arg->setNumber(arg->number() + by);
	}

	updateLook();
}


int MathMacroTemplate::maxArgumentInDefinition() const
{
	// We don't have a buffer when pasting from the clipboard (bug 6014).
	Buffer const * macro_buffer = isBufferLoaded() ? &buffer() : 0;
	int maxArg = 0;
	DocIterator it = doc_iterator_begin(macro_buffer, this);
	it.idx() = defIdx();
	for (; it; it.forwardChar()) {
		if (!it.nextInset())
			continue;
		if (it.nextInset()->lyxCode() != MATH_MACROARG_CODE)
			continue;
		MathMacroArgument * arg = static_cast<MathMacroArgument*>(it.nextInset());
		maxArg = std::max(int(arg->number()), maxArg);
	}
	return maxArg;
}


void MathMacroTemplate::insertMissingArguments(int maxArg)
{
	bool found[9] = { false, false, false, false, false, false, false, false, false };
	idx_type idx = cell(displayIdx()).empty() ? defIdx() : displayIdx();

	// search for #n macros arguments
	DocIterator it = doc_iterator_begin(&buffer(), this);
	it.idx() = idx;
	for (; it && it[0].idx() == idx; it.forwardChar()) {
		if (!it.nextInset())
			continue;
		if (it.nextInset()->lyxCode() != MATH_MACROARG_CODE)
			continue;
		MathMacroArgument * arg = static_cast<MathMacroArgument*>(it.nextInset());
		found[arg->number() - 1] = true;
	}

	// add missing ones
	for (int i = 0; i < maxArg; ++i) {
		if (found[i])
			continue;

		cell(idx).push_back(MathAtom(new MathMacroArgument(i + 1)));
	}
}


void MathMacroTemplate::changeArity(Cursor & cur,
	DocIterator const & inset_pos, int newNumArg)
{
	// remove parameter which do not appear anymore in the definition
	for (int i = numargs_; i > newNumArg; --i)
		removeParameter(cur, inset_pos, numargs_ - 1, true);
	
	// add missing parameter
	for (int i = numargs_; i < newNumArg; ++i)
		insertParameter(cur, inset_pos, numargs_, true, false);
}


///
class AddRemoveMacroInstanceFix
{
public:
	///
	AddRemoveMacroInstanceFix(int n, bool insert) : n_(n), insert_(insert) {}
	///
	void operator()(MathMacro * macro)
	{
		if (macro->folded()) {
			if (insert_)
				macro->insertArgument(n_);
			else
				macro->removeArgument(n_);
		}
	}

private:
	///
	int n_;
	///
	bool insert_;
};


///
class OptionalsMacroInstanceFix
{
public:
	///
	OptionalsMacroInstanceFix(int optionals) : optionals_(optionals) {}
	///
	void operator()(MathMacro * macro)
	{
		macro->setOptionals(optionals_);
	}

private:
	///
	int optionals_;
};


///
class NullMacroInstanceFix
{
public:
	///
	void operator()(MathMacro * ) {}
};


template<class F>
void fixMacroInstances(Cursor & cur, DocIterator const & inset_pos,
	docstring const & name, F & fix)
{
	// goto position behind macro template
	DocIterator dit = inset_pos;
	dit.pop_back();
	dit.top().forwardPos();

	// remember hull to trigger preview reload
	DocIterator hull(dit.buffer());
	bool preview_reload_needed = false;
	set<DocIterator> preview_hulls;

	// iterate over all positions until macro is redefined
	for (; dit; dit.forwardPos()) {
		// left the outer hull?
		if (!hull.empty() && dit.depth() == hull.depth()) {
			// schedule reload of the preview if necessary 
			if (preview_reload_needed) {
				preview_hulls.insert(hull);
				preview_reload_needed = false;
			}
			hull.clear();
		}

		// only until a macro is redefined
		if (dit.inset().lyxCode() == MATHMACRO_CODE) {
			MathMacroTemplate const & macroTemplate
			= static_cast<MathMacroTemplate const &>(dit.inset());
			if (macroTemplate.name() == name)
				break;
		}

		// in front of macro instance?
		Inset * inset = dit.nextInset();
		if (!inset)
			continue;
		InsetMath * insetMath = inset->asInsetMath();
		if (!insetMath)
			continue;

		// in front of outer hull?
		InsetMathHull * inset_hull = insetMath->asHullInset();
		if (inset_hull && hull.empty()) {
			// remember this for later preview reload
			hull = dit;
		}

		MathMacro * macro = insetMath->asMacro();
		if (macro && macro->name() == name && macro->folded()) {
			fix(macro);
			if (RenderPreview::previewMath())
				preview_reload_needed = true;
		}
	}

	if (!preview_hulls.empty()) {
		// reload the scheduled previews
		set<DocIterator>::const_iterator sit = preview_hulls.begin();
		set<DocIterator>::const_iterator end = preview_hulls.end();
		for (; sit != end; ++sit) {
			InsetMathHull * inset_hull =
				sit->nextInset()->asInsetMath()->asHullInset();
			LBUFERR(inset_hull);
			inset_hull->reloadPreview(*sit);
		}
		cur.screenUpdateFlags(Update::Force);
	}
}


void MathMacroTemplate::commitEditChanges(Cursor & cur,
	DocIterator const & inset_pos)
{
	int args_in_def = maxArgumentInDefinition();
	if (args_in_def != numargs_) {
		// FIXME: implement precise undo handling (only a few places
		//   need undo)
		cur.recordUndoFullBuffer();
		changeArity(cur, inset_pos, args_in_def);
	}
	insertMissingArguments(args_in_def);

	// make sure the preview are up to date
	NullMacroInstanceFix fix;
	fixMacroInstances(cur, inset_pos, name(), fix);
}


void MathMacroTemplate::insertParameter(Cursor & cur,
	DocIterator const & inset_pos, int pos, bool greedy, bool addarg)
{
	if (pos <= numargs_ && pos >= optionals_ && numargs_ < 9) {
		++numargs_;

		// append example #n
		if (addarg) {
			shiftArguments(pos, 1);

			cell(defIdx()).push_back(MathAtom(new MathMacroArgument(pos + 1)));
			if (!cell(displayIdx()).empty())
				cell(displayIdx()).push_back(MathAtom(new MathMacroArgument(pos + 1)));
		}

		if (!greedy) {
			// fix macro instances
			AddRemoveMacroInstanceFix fix(pos, true);
			fixMacroInstances(cur, inset_pos, name(), fix);
		}
	}

	updateLook();
}


void MathMacroTemplate::removeParameter(Cursor & cur,
	DocIterator const & inset_pos, int pos, bool greedy)
{
	if (pos < numargs_ && pos >= 0) {
		--numargs_;
		removeArguments(cur, inset_pos, pos, pos);
		shiftArguments(pos + 1, -1);

		// removed optional parameter?
		if (pos < optionals_) {
			--optionals_;
			optionalValues_[pos] = cell(optIdx(pos));
			cells_.erase(cells_.begin() + optIdx(pos));

			// fix cursor
			int macroSlice = cur.find(this);
			if (macroSlice != -1) {
				if (cur[macroSlice].idx() == optIdx(pos)) {
					cur.cutOff(macroSlice);
					cur[macroSlice].idx() = 1;
					cur[macroSlice].pos() = 0;
				} else if (cur[macroSlice].idx() > optIdx(pos))
					--cur[macroSlice].idx();
			}
		}

		if (!greedy) {
			// fix macro instances
			AddRemoveMacroInstanceFix fix(pos, false);
			fixMacroInstances(cur, inset_pos, name(), fix);
		}
	}

	updateLook();
}


void MathMacroTemplate::makeOptional(Cursor & cur,
	DocIterator const & inset_pos)
{
	if (numargs_ > 0 && optionals_ < numargs_) {
		++optionals_;
		cells_.insert(cells_.begin() + optIdx(optionals_ - 1), optionalValues_[optionals_ - 1]);
		// fix cursor
		int macroSlice = cur.find(this);
		if (macroSlice != -1 && cur[macroSlice].idx() >= optIdx(optionals_ - 1))
			++cur[macroSlice].idx();

		// fix macro instances
		OptionalsMacroInstanceFix fix(optionals_);
		fixMacroInstances(cur, inset_pos, name(), fix);
	}

	updateLook();
}


void MathMacroTemplate::makeNonOptional(Cursor & cur,
	DocIterator const & inset_pos)
{
	if (numargs_ > 0 && optionals_ > 0) {
		--optionals_;

		// store default value for later if the user changes his mind
		optionalValues_[optionals_] = cell(optIdx(optionals_));
		cells_.erase(cells_.begin() + optIdx(optionals_));

		// fix cursor
		int macroSlice = cur.find(this);
		if (macroSlice != -1) {
			if (cur[macroSlice].idx() > optIdx(optionals_))
				--cur[macroSlice].idx();
			else if (cur[macroSlice].idx() == optIdx(optionals_)) {
				cur.cutOff(macroSlice);
				cur[macroSlice].idx() = optIdx(optionals_);
				cur[macroSlice].pos() = 0;
			}
		}

		// fix macro instances
		OptionalsMacroInstanceFix fix(optionals_);
		fixMacroInstances(cur, inset_pos, name(), fix);
	}

	updateLook();
}


void MathMacroTemplate::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	string const arg = to_utf8(cmd.argument());
	switch (cmd.action()) {

	case LFUN_MATH_MACRO_ADD_PARAM:
		if (numargs_ < 9) {
			commitEditChanges(cur, cur);
			// FIXME: implement precise undo handling (only a few places
			//   need undo)
			cur.recordUndoFullBuffer();
			size_t pos = numargs_;
			if (!arg.empty())
				pos = (size_t)convert<int>(arg) - 1; // it is checked for >=0 in getStatus
			insertParameter(cur, cur, pos);
		}
		break;


	case LFUN_MATH_MACRO_REMOVE_PARAM:
		if (numargs_ > 0) {
			commitEditChanges(cur, cur);
			// FIXME: implement precise undo handling (only a few places
			//   need undo)
			cur.recordUndoFullBuffer();
			size_t pos = numargs_ - 1;
			if (!arg.empty())
				pos = (size_t)convert<int>(arg) - 1; // it is checked for >=0 in getStatus
			removeParameter(cur, cur, pos);
		}
		break;

	case LFUN_MATH_MACRO_APPEND_GREEDY_PARAM:
		if (numargs_ < 9) {
			commitEditChanges(cur, cur);
			// FIXME: implement precise undo handling (only a few places
			//   need undo)
			cur.recordUndoFullBuffer();
			insertParameter(cur, cur, numargs_, true);
		}
		break;

	case LFUN_MATH_MACRO_REMOVE_GREEDY_PARAM:
		if (numargs_ > 0) {
			commitEditChanges(cur, cur);
			// FIXME: implement precise undo handling (only a few places
			//   need undo)
			cur.recordUndoFullBuffer();
			removeParameter(cur, cur, numargs_ - 1, true);
		}
		break;

	case LFUN_MATH_MACRO_MAKE_OPTIONAL:
		commitEditChanges(cur, cur);
		// FIXME: implement precise undo handling (only a few places
		//   need undo)
		cur.recordUndoFullBuffer();
		makeOptional(cur, cur);
		break;

	case LFUN_MATH_MACRO_MAKE_NONOPTIONAL:
		commitEditChanges(cur, cur);
		// FIXME: implement precise undo handling (only a few places
		//   need undo)
		cur.recordUndoFullBuffer();
		makeNonOptional(cur, cur);
		break;

	case LFUN_MATH_MACRO_ADD_OPTIONAL_PARAM:
		if (numargs_ < 9) {
			commitEditChanges(cur, cur);
			// FIXME: implement precise undo handling (only a few places
			//   need undo)
			cur.recordUndoFullBuffer();
			insertParameter(cur, cur, optionals_);
			makeOptional(cur, cur);
		}
		break;

	case LFUN_MATH_MACRO_REMOVE_OPTIONAL_PARAM:
		if (optionals_ > 0) {
			commitEditChanges(cur, cur);
			// FIXME: implement precise undo handling (only a few places
			//   need undo)
			cur.recordUndoFullBuffer();
			removeParameter(cur, cur, optionals_ - 1);
		} break;

	case LFUN_MATH_MACRO_ADD_GREEDY_OPTIONAL_PARAM:
		if (numargs_ == optionals_) {
			commitEditChanges(cur, cur);
			// FIXME: implement precise undo handling (only a few places
			//   need undo)
			cur.recordUndoFullBuffer();
			insertParameter(cur, cur, 0, true);
			makeOptional(cur, cur);
		}
		break;

	default:
		InsetMathNest::doDispatch(cur, cmd);
		break;
	}
}


bool MathMacroTemplate::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & flag) const
{
	bool ret = true;
	string const arg = to_utf8(cmd.argument());
	switch (cmd.action()) {
		case LFUN_MATH_MACRO_ADD_PARAM: {
			int num = numargs_ + 1;
			if (!arg.empty())
				num = convert<int>(arg);
			bool on = (num >= optionals_
				   && numargs_ < 9 && num <= numargs_ + 1);
			flag.setEnabled(on);
			break;
		}

		case LFUN_MATH_MACRO_APPEND_GREEDY_PARAM:
			flag.setEnabled(numargs_ < 9);
			break;

		case LFUN_MATH_MACRO_REMOVE_GREEDY_PARAM:
		case LFUN_MATH_MACRO_REMOVE_PARAM: {
			int num = numargs_;
			if (!arg.empty())
				num = convert<int>(arg);
			flag.setEnabled(num >= 1 && num <= numargs_);
			break;
		}

		case LFUN_MATH_MACRO_MAKE_OPTIONAL:
			flag.setEnabled(numargs_ > 0
				     && optionals_ < numargs_
				     && type_ != MacroTypeDef);
			break;

		case LFUN_MATH_MACRO_MAKE_NONOPTIONAL:
			flag.setEnabled(optionals_ > 0
				     && type_ != MacroTypeDef);
			break;

		case LFUN_MATH_MACRO_ADD_OPTIONAL_PARAM:
			flag.setEnabled(numargs_ < 9);
			break;

		case LFUN_MATH_MACRO_REMOVE_OPTIONAL_PARAM:
			flag.setEnabled(optionals_ > 0);
			break;

		case LFUN_MATH_MACRO_ADD_GREEDY_OPTIONAL_PARAM:
			flag.setEnabled(numargs_ == 0
				     && type_ != MacroTypeDef);
			break;

		case LFUN_IN_MATHMACROTEMPLATE:
			flag.setEnabled(true);
			break;

		default:
			ret = InsetMathNest::getStatus(cur, cmd, flag);
			break;
	}
	return ret;
}


void MathMacroTemplate::read(Lexer & lex)
{
	MathData ar(buffer_);
	mathed_parse_cell(ar, lex.getStream(), Parse::TRACKMACRO);
	if (ar.size() != 1 || !ar[0]->asMacroTemplate()) {
		lyxerr << "Cannot read macro from '" << ar << "'" << endl;
		lyxerr << "Read: " << to_utf8(asString(ar)) << endl;
		return;
	}
	operator=( *(ar[0]->asMacroTemplate()) );

	updateLook();
}


void MathMacroTemplate::write(ostream & os) const
{
	odocstringstream oss;
	TexRow texrow(false);
	otexrowstream ots(oss,texrow);
	WriteStream wi(ots, false, false, WriteStream::wsDefault);
	oss << "FormulaMacro\n";
	write(wi);
	os << to_utf8(oss.str());
}


void MathMacroTemplate::write(WriteStream & os) const
{
	write(os, false);
}


int MathMacroTemplate::write(WriteStream & os, bool overwriteRedefinition) const
{
	int num_lines = 0;

	if (os.latex()) {
		if (optionals_ > 0) {
			// macros with optionals use the xargs package, e.g.:
			// \newcommandx{\foo}[2][usedefault, addprefix=\global,1=default]{#1,#2}
			// \long is implicit by xargs
			if (redefinition_ && !overwriteRedefinition)
				os << "\\renewcommandx";
			else
				os << "\\newcommandx";

			os << "\\" << name()
			   << "[" << numargs_ << "]"
			   << "[usedefault, addprefix=\\global";
			for (int i = 0; i < optionals_; ++i) {
				docstring optValue = asString(cell(optIdx(i)));
				if (optValue.find(']') != docstring::npos
				    || optValue.find(',') != docstring::npos)
					os << ", " << i + 1 << "="
					<< "{" << cell(optIdx(i)) << "}";
				else
					os << ", " << i + 1 << "="
					<< cell(optIdx(i));
			}
			os << "]";
		} else {
			// Macros without optionals use standard _global_ \def macros:
			//   \global\def\long\foo#1#2{#1,#2}
			// We use the \long prefix as this is the equivalent to \newcommand.
			// We cannot use \newcommand directly because \global does not work with it.
			os << "\\global\\long\\def\\" << name();
			docstring param = from_ascii("#0");
			for (int i = 1; i <= numargs_; ++i) { 
				param[1] = '0' + i;
				os << param;
			}
		}
	} else {
		// in LyX output we use some pseudo syntax which is implementation
		// independent, e.g.
		// \newcommand{\foo}[2][default]{#1,#2}
		if (redefinition_ && !overwriteRedefinition)
			os << "\\renewcommand";
		else
			os << "\\newcommand";
		os << "{\\" << name() << '}';
		if (numargs_ > 0)
			os << '[' << numargs_ << ']';

		for (int i = 0; i < optionals_; ++i) {
			docstring optValue = asString(cell(optIdx(i)));
			if (optValue.find(']') != docstring::npos)
				os << "[{" << cell(optIdx(i)) << "}]";
			else
				os << "[" << cell(optIdx(i)) << "]";
		}
	}

	os << "{" << cell(defIdx()) << "}";

	if (os.latex()) {
		// writing .tex. done.
		os << "\n";
		++num_lines;
	} else {
		// writing .lyx, write special .tex export only if necessary
		if (!cell(displayIdx()).empty()) {
			os << "\n{" << cell(displayIdx()) << '}';
			++num_lines;
		}
	}

	return num_lines;
}


docstring MathMacroTemplate::xhtml(XHTMLStream &, OutputParams const &) const
{
	return docstring();
}

int MathMacroTemplate::plaintext(odocstringstream & os,
				 OutputParams const &, size_t) const
{
	docstring const str = '[' + buffer().B_("math macro") + ']';

	os << str;
	return str.size();
}


bool MathMacroTemplate::validName() const
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
	for (size_t i = 0; i < n.size(); ++i) {
		if (!(n[i] >= 'a' && n[i] <= 'z')
		    && !(n[i] >= 'A' && n[i] <= 'Z')
		    && n[i] != '*')
			return false;
	}

	return true;
}


bool MathMacroTemplate::validMacro() const
{
	return validName();
}


bool MathMacroTemplate::fixNameAndCheckIfValid()
{
	// check all the characters/insets in the name cell
	size_t i = 0;
	MathData & data = cell(0);
	while (i < data.size()) {
		InsetMathChar const * cinset = data[i]->asCharInset();
		if (cinset) {
			// valid character in [a-zA-Z]?
			char_type c = cinset->getChar();
			if ((c >= 'a' && c <= 'z')
			    || (c >= 'A' && c <= 'Z')) {
				++i;
				continue;
			}
		}

		// throw cell away
		data.erase(i);
	}

	// now it should be valid if anything in the name survived
	return !data.empty();
}

	
void MathMacroTemplate::validate(LaTeXFeatures & features) const
{
	// we need global optional macro arguments. They are not available 
	// with \def, and \newcommand does not support global macros. So we
	// are bound to xargs also for the single-optional-parameter case.
	if (optionals_ > 0)
		features.require("xargs");
}

void MathMacroTemplate::getDefaults(vector<docstring> & defaults) const
{
	defaults.resize(numargs_);
	for (int i = 0; i < optionals_; ++i)
		defaults[i] = asString(cell(optIdx(i)));
}


docstring MathMacroTemplate::definition() const
{
	return asString(cell(defIdx()));
}


docstring MathMacroTemplate::displayDefinition() const
{
	return asString(cell(displayIdx()));
}


size_t MathMacroTemplate::numArgs() const
{
	return numargs_;
}


size_t MathMacroTemplate::numOptionals() const
{
	return optionals_;
}


void MathMacroTemplate::infoize(odocstream & os) const
{
	os << bformat(_("Math Macro: \\%1$s"), name());
}


string MathMacroTemplate::contextMenuName() const
{
	return "context-math-macro-definition";
}

void MathMacroTemplate::addToToc(DocIterator const & pit, bool output_active,
								 UpdateType) const
{
	shared_ptr<Toc> toc = buffer().tocBackend().toc("math-macro");
	docstring str;
	if (!validMacro())
		str = bformat(_("Invalid macro! \\%1$s"), name());
	else
		str = "\\" + name();
	toc->push_back(TocItem(pit, 0, str, output_active));
}


} // namespace lyx
