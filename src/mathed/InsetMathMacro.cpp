/**
 * \file InsetMathMacro.cpp
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

#include "InsetMathMacro.h"

#include "InsetMathChar.h"
#include "InsetMathScript.h"
#include "MathCompletionList.h"
#include "MathExtern.h"
#include "MathFactory.h"
#include "MathRow.h"
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
#include "MetricsInfo.h"

#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/Changer.h"
#include "support/textutils.h"

#include <ostream>
#include <vector>

using namespace lyx::support;
using namespace std;

namespace lyx {


/// A proxy for the macro values
class InsetArgumentProxy : public InsetMath {
public:
	///
	InsetArgumentProxy(InsetMathMacro * mathMacro, size_t idx)
		: mathMacro_(mathMacro), idx_(idx) {}
	///
	InsetArgumentProxy(InsetMathMacro * mathMacro, size_t idx, docstring const & def)
		: mathMacro_(mathMacro), idx_(idx)
	{
			asArray(def, def_);
	}
	///
	void setBuffer(Buffer & buffer) override
	{
		Inset::setBuffer(buffer);
		def_.setBuffer(buffer);
	}
	///
	void setOwner(InsetMathMacro * mathMacro) { mathMacro_ = mathMacro; }
	///
	InsetMathMacro const * owner() { return mathMacro_; }
	///
	marker_type marker(BufferView const *) const override { return marker_type::NO_MARKER; }
	///
	InsetCode lyxCode() const override { return ARGUMENT_PROXY_CODE; }
	/// The math data to use for display
	MathData const & displayCell(BufferView const * bv) const
	{
		// handle default macro arguments
		bool use_def_arg = !mathMacro_->editMetrics(bv)
			&& mathMacro_->cell(idx_).empty();
		return use_def_arg ? def_ : mathMacro_->cell(idx_);
	}
	///
	bool addToMathRow(MathRow & mrow, MetricsInfo & mi) const override
	{
		// macro arguments are in macros
		LATTEST(mathMacro_->nesting() > 0);
		/// The macro nesting can change display of insets. Change it locally.
		Changer chg = changeVar(mi.base.macro_nesting,
		                          mathMacro_->nesting() == 1 ? 0 : mathMacro_->nesting());

		MathRow::Element e_beg(mi, MathRow::BEGIN);
		e_beg.inset = this;
		e_beg.ar = &mathMacro_->cell(idx_);
		mrow.push_back(e_beg);

		mathMacro_->macro()->unlock();
		bool has_contents = displayCell(mi.base.bv).addToMathRow(mrow, mi);
		mathMacro_->macro()->lock();

		// if there was no contents, and the contents is editable,
		// then we insert a box instead.
		if (!has_contents && mathMacro_->nesting() == 1) {
			// mathclass is ord because it should be spaced as a normal atom
			MathRow::Element e(mi, MathRow::BOX, MC_ORD);
			e.color = Color_mathline;
			mrow.push_back(e);
			has_contents = true;
		}

		MathRow::Element e_end(mi, MathRow::END);
		e_end.inset = this;
		e_end.ar = &mathMacro_->cell(idx_);
		mrow.push_back(e_end);

		return has_contents;
	}
	///
	void beforeMetrics() const override
	{
		mathMacro_->macro()->unlock();
	}
	///
	void afterMetrics() const override
	{
		mathMacro_->macro()->lock();
	}
	///
	void beforeDraw(PainterInfo const & pi) const override
	{
		// if the macro is being edited, then the painter is in
		// monochrome mode.
		if (mathMacro_->editMetrics(pi.base.bv))
			pi.pain.leaveMonochromeMode();
	}
	///
	void afterDraw(PainterInfo const & pi) const override
	{
		if (mathMacro_->editMetrics(pi.base.bv))
			pi.pain.enterMonochromeMode(Color_mathmacroblend);
	}
	///
	void metrics(MetricsInfo &, Dimension &) const override {
		// This should never be invoked, since InsetArgumentProxy insets are linearized
		LATTEST(false);
	}
	///
	void draw(PainterInfo &, int, int) const override {
		// This should never be invoked, since InsetArgumentProxy insets are linearized
		LATTEST(false);
	}
	///
	int kerning(BufferView const * bv) const override
	{
		return displayCell(bv).kerning(bv);
	}
	// write(), normalize(), infoize() and infoize2() are not needed since
	// InsetMathMacro uses the definition and not the expanded cells.
	///
	void maple(MapleStream & ms) const override { ms << mathMacro_->cell(idx_); }
	///
	void maxima(MaximaStream & ms) const override { ms << mathMacro_->cell(idx_); }
	///
	void mathematica(MathematicaStream & ms) const override { ms << mathMacro_->cell(idx_); }
	///
	void mathmlize(MathMLStream & ms) const override { ms << mathMacro_->cell(idx_); }
	///
	void htmlize(HtmlStream & ms) const override { ms << mathMacro_->cell(idx_); }
	///
	void octave(OctaveStream & os) const override { os << mathMacro_->cell(idx_); }
	///
	MathClass mathClass() const override
	{
		return MC_UNKNOWN;
		// This can be refined once the pointer issues are fixed. I did not
		// notice any immediate crash with the following code, but it is risky
		// nevertheless:
		//return mathMacro_->cell(idx_).mathClass();
	}

private:
	///
	Inset * clone() const override
	{
		return new InsetArgumentProxy(*this);
	}
	///
	InsetMathMacro * mathMacro_;
	///
	size_t idx_;
	///
	MathData def_;
};


/// Private implementation of InsetMathMacro
class InsetMathMacro::Private {
public:
	Private(Buffer * buf, docstring const & name)
		: name_(name), displayMode_(DISPLAY_INIT),
		  expanded_(buf), definition_(buf), attachedArgsNum_(0),
		  optionals_(0), nextFoldMode_(true), macroBackup_(buf),
		  macro_(nullptr), needsUpdate_(false), isUpdating_(false),
		  appetite_(9), nesting_(0), limits_(AUTO_LIMITS)
	{
	}
	/// Update the pointers to our owner of all expanded macros.
	/// This needs to be called every time a copy of the owner is created
	/// (bug 9418).
	void updateChildren(InsetMathMacro * owner);
	/// Recursively update the pointers of all expanded macros
	/// appearing in the arguments of the current macro
	void updateNestedChildren(InsetMathMacro * owner, InsetMathNest * ni);
	/// name of macro
	docstring name_;
	/// current display mode
	DisplayMode displayMode_;
	/// expanded macro with ArgumentProxies
	MathData expanded_;
	/// macro definition with #1,#2,.. insets
	MathData definition_;
	/// number of arguments that were really attached
	size_t attachedArgsNum_;
	/// optional argument attached? (only in DISPLAY_NORMAL mode)
	size_t optionals_;
	/// fold mode to be set in next metrics call?
	bool nextFoldMode_;
	/// if macro_ == true, then here is a copy of the macro
	/// don't use it for locking
	MacroData macroBackup_;
	/// if macroNotFound_ == false, then here is a reference to the macro
	/// this might invalidate after metrics was called
	MacroData const * macro_;
	///
	mutable std::map<BufferView const *, bool> editing_;
	///
	std::string required_;
	/// update macro representation
	bool needsUpdate_;
	///
	bool isUpdating_;
	/// maximal number of arguments the macro is greedy for
	size_t appetite_;
	/// Level of nesting in macros (including this one)
	int nesting_;
	///
	Limits limits_;
};


void InsetMathMacro::Private::updateChildren(InsetMathMacro * owner)
{
	for (size_t i = 0; i < expanded_.size(); ++i) {
		InsetArgumentProxy * p = dynamic_cast<InsetArgumentProxy *>(expanded_[i].nucleus());
		if (p)
			p->setOwner(owner);

		InsetMathNest * ni = expanded_[i].nucleus()->asNestInset();
		if (ni)
			updateNestedChildren(owner, ni);
	}

	if (macro_) {
		// The macro_ pointer is updated when MathData::metrics() is
		// called. However, when instant preview is on or the macro is
		// not on screen, MathData::metrics() is not called and we may
		// have a dangling pointer. As a safety measure, when a macro
		// is copied, always let macro_ point to the backup copy of the
		// MacroData structure. This backup is updated every time the
		// macro is changed, so it will not become stale.
		macro_ = &macroBackup_;
	}
}


void InsetMathMacro::Private::updateNestedChildren(InsetMathMacro * owner, InsetMathNest * ni)
{
	for (size_t i = 0; i < ni->nargs(); ++i) {
		MathData & ar = ni->cell(i);
		for (size_t j = 0; j < ar.size(); ++j) {
			InsetArgumentProxy * ap = dynamic_cast
				<InsetArgumentProxy *>(ar[j].nucleus());
			if (ap) {
				InsetMathMacro::Private * md = ap->owner()->d;
				if (md->macro_)
					md->macro_ = &md->macroBackup_;
				ap->setOwner(owner);
			}
			InsetMathNest * imn = ar[j].nucleus()->asNestInset();
			if (imn)
				updateNestedChildren(owner, imn);
		}
	}
}


InsetMathMacro::InsetMathMacro(Buffer * buf, docstring const & name)
	: InsetMathNest(buf, 0), d(new Private(buf, name))
{}


InsetMathMacro::InsetMathMacro(InsetMathMacro const & that)
	: InsetMathNest(that), d(new Private(*that.d))
{
	// FIXME This should not really be necessary, but when we are
	// initializing the table of global macros, we create macros
	// with no associated Buffer.
	if (that.buffer_)
		setBuffer(*that.buffer_);
	d->updateChildren(this);
}


InsetMathMacro & InsetMathMacro::operator=(InsetMathMacro const & that)
{
	if (&that == this)
		return *this;
	InsetMathNest::operator=(that);
	*d = *that.d;
	d->updateChildren(this);
	return *this;
}


InsetMathMacro::~InsetMathMacro()
{
	delete d;
}


bool InsetMathMacro::addToMathRow(MathRow & mrow, MetricsInfo & mi) const
{
	// set edit mode for which we will have calculated row.
	// This is the same as what is done in metrics().
	d->editing_[mi.base.bv] = editMode(mi.base.bv);

	// For now we do not linearize in the following cases (can be improved)
	// - display mode different from normal
	// - editing with parameter list
	// - editing with box around macro
	if (displayMode() != InsetMathMacro::DISPLAY_NORMAL
		|| (d->editing_[mi.base.bv] && lyxrc.macro_edit_style == LyXRC::MACRO_EDIT_LIST))
		return InsetMath::addToMathRow(mrow, mi);

	/// The macro nesting can change display of insets. Change it locally.
	Changer chg = changeVar(mi.base.macro_nesting, d->nesting_);

	MathRow::Element e_beg(mi, MathRow::BEGIN);
	e_beg.inset = this;
	e_beg.marker = (d->nesting_ == 1) ? marker(mi.base.bv) : marker_type::NO_MARKER;
	mrow.push_back(e_beg);

	d->macro_->lock();
	bool has_contents = d->expanded_.addToMathRow(mrow, mi);
	d->macro_->unlock();

	// if there was no contents and the array is editable, then we
	// insert a grey box instead.
	if (!has_contents && mi.base.macro_nesting == 1) {
		// mathclass is unknown because it is irrelevant for spacing
		MathRow::Element e(mi, MathRow::BOX);
		e.color = Color_mathmacroblend;
		mrow.push_back(e);
		has_contents = true;
	}

	MathRow::Element e_end(mi, MathRow::END);
	e_end.inset = this;
	e_end.marker = (d->nesting_ == 1) ? marker(mi.base.bv) : marker_type::NO_MARKER;
	mrow.push_back(e_end);

	return has_contents;
}


/// Whether the inset allows \(no)limits
bool InsetMathMacro::allowsLimitsChange() const
{
	// similar to the code in mathClass(), except that we search for
	// the right-side class.
	MathClass mc = MC_UNKNOWN;
	if (MacroData const * m = macroBackup()) {
		// If it is a global macro and is defined explicitly
		if (m->symbol())
			mc = string_to_class(m->symbol()->extra);
	}
	// Otherwise guess from the expanded macro
	if (mc == MC_UNKNOWN)
		mc = d->expanded_.lastMathClass();

	return mc == MC_OP;
}


Limits InsetMathMacro::defaultLimits(bool display) const
{
	if (d->expanded_.empty())
		return NO_LIMITS;
	// Guess from the expanded macro
	InsetMath const * in = d->expanded_.back().nucleus();
	Limits const lim = in->limits() == AUTO_LIMITS
		? in->defaultLimits(display) : in->limits();
	LATTEST(lim != AUTO_LIMITS);
	return lim;
}


Limits InsetMathMacro::limits() const
{
	return d->limits_;
}


void InsetMathMacro::limits(Limits lim)
{
	d->limits_ = lim;
}


void InsetMathMacro::beforeMetrics() const
{
	d->macro_->lock();
}


void InsetMathMacro::afterMetrics() const
{
	d->macro_->unlock();
}


void InsetMathMacro::beforeDraw(PainterInfo const & pi) const
{
	if (d->editing_[pi.base.bv])
		pi.pain.enterMonochromeMode(Color_mathmacroblend);
}


void InsetMathMacro::afterDraw(PainterInfo const & pi) const
{
	if (d->editing_[pi.base.bv])
		pi.pain.leaveMonochromeMode();
}


Inset * InsetMathMacro::clone() const
{
	InsetMathMacro * copy = new InsetMathMacro(*this);
	copy->d->needsUpdate_ = true;
	//copy->d->expanded_.clear();
	return copy;
}


void InsetMathMacro::normalize(NormalStream & os) const
{
	os << "[macro " << name();
	for (size_t i = 0; i < nargs(); ++i)
		os << ' ' << cell(i);
	os << ']';
}


InsetMathMacro::DisplayMode InsetMathMacro::displayMode() const
{
	return d->displayMode_;
}


bool InsetMathMacro::extraBraces() const
{
	return d->displayMode_ == DISPLAY_NORMAL && arity() > 0;
}


docstring InsetMathMacro::name() const
{
	if (d->displayMode_ == DISPLAY_UNFOLDED)
		return asString(cell(0));

	return d->name_;
}


docstring InsetMathMacro::macroName() const
{
	return d->name_;
}


int InsetMathMacro::nesting() const
{
	return d->nesting_;
}


void InsetMathMacro::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x,	int & y) const
{
	// We may have 0 arguments, but InsetMathNest requires at least one.
	if (nargs() > 0)
		InsetMathNest::cursorPos(bv, sl, boundary, x, y);
}


bool InsetMathMacro::editMode(BufferView const * bv) const {
	// find this in cursor trace
	Cursor const & cur = bv->cursor();
	for (size_t i = 0; i != cur.depth(); ++i)
		if (&cur[i].inset() == this) {
			// look if there is no other macro in edit mode above
			++i;
			for (; i != cur.depth(); ++i) {
				InsetMath * im = cur[i].asInsetMath();
				if (im) {
					InsetMathMacro const * macro = im->asMacro();
					if (macro && macro->displayMode() == DISPLAY_NORMAL)
						return false;
				}
			}

			// ok, none found, I am the highest one
			return true;
		}

	return false;
}


MacroData const * InsetMathMacro::macro() const
{
	return d->macro_;
}


bool InsetMathMacro::editMetrics(BufferView const * bv) const
{
	return d->editing_[bv];
}


marker_type InsetMathMacro::marker(BufferView const * bv) const
{
	if (nargs() == 0)
		return marker_type::NO_MARKER;

	switch (d->displayMode_) {
	case DISPLAY_INIT:
	case DISPLAY_INTERACTIVE_INIT:
		return marker_type::NO_MARKER;
	case DISPLAY_UNFOLDED:
		return marker_type::MARKER;
	case DISPLAY_NORMAL:
		switch (lyxrc.macro_edit_style) {
		case LyXRC::MACRO_EDIT_INLINE:
			return marker_type::MARKER2;
		case LyXRC::MACRO_EDIT_INLINE_BOX:
			return d->editing_[bv] ? marker_type::BOX_MARKER : marker_type::MARKER2;
		case LyXRC::MACRO_EDIT_LIST:
			return marker_type::MARKER2;
		}
	}
	// please gcc 4.6
	return marker_type::NO_MARKER;
}


void InsetMathMacro::metrics(MetricsInfo & mi, Dimension & dim) const
{
	/// The macro nesting can change display of insets. Change it locally.
	Changer chg = changeVar(mi.base.macro_nesting, d->nesting_);

	// set edit mode for which we will have calculated metrics. But only
	d->editing_[mi.base.bv] = editMode(mi.base.bv);

	// calculate new metrics according to display mode
	if (d->displayMode_ == DISPLAY_INIT || d->displayMode_ == DISPLAY_INTERACTIVE_INIT) {
		Changer dummy = mi.base.changeFontSet("lyxtex");
		mathed_string_dim(mi.base.font, from_ascii("\\") + name(), dim);
	} else if (d->displayMode_ == DISPLAY_UNFOLDED) {
		Changer dummy = mi.base.changeFontSet("lyxtex");
		cell(0).metrics(mi, dim);
		Dimension bsdim;
		mathed_string_dim(mi.base.font, from_ascii("\\"), bsdim);
		dim.wid += bsdim.width() + 1;
		dim.asc = max(bsdim.ascent(), dim.ascent());
		dim.des = max(bsdim.descent(), dim.descent());
	} else if (lyxrc.macro_edit_style == LyXRC::MACRO_EDIT_LIST
		   && d->editing_[mi.base.bv]) {
		// Macro will be edited in a old-style list mode here:

		LBUFERR(d->macro_);
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
		d->definition_.metrics(mi, defDim);

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
	} else {
		// We should not be here, since the macro is linearized in this case.
		LBUFERR(false);
	}
}


int InsetMathMacro::kerning(BufferView const * bv) const {
	if (d->displayMode_ == DISPLAY_NORMAL && !d->editing_[bv])
		return d->expanded_.kerning(bv);
	else
		return 0;
}


void InsetMathMacro::updateMacro(MacroContext const & mc)
{
	if (validName()) {
		d->macro_ = mc.get(name());
		if (d->macro_ && d->macroBackup_ != *d->macro_) {
			d->macroBackup_ = *d->macro_;
			d->needsUpdate_ = true;
		}
	} else {
		d->macro_ = nullptr;
	}
}


class InsetMathMacro::UpdateLocker
{
public:
	explicit UpdateLocker(InsetMathMacro & mm) : mac(mm)
	{
		mac.d->isUpdating_ = true;
	}
	~UpdateLocker() { mac.d->isUpdating_ = false; }
private:
	InsetMathMacro & mac;
};
/** Avoid wrong usage of UpdateLocker.
    To avoid wrong usage:
    UpdateLocker(...); // wrong
    UpdateLocker locker(...); // right
*/
#define UpdateLocker(x) unnamed_UpdateLocker;
// Tip gotten from Bobby Schmidt's column in C/C++ Users Journal


void InsetMathMacro::updateRepresentation(Cursor * cur, MacroContext const & mc,
	UpdateType utype, int nesting)
{
	// block recursive calls (bug 8999)
	if (d->isUpdating_)
		return;

	UpdateLocker locker(*this);

	// known macro?
	if (d->macro_ == nullptr)
		return;

	// remember nesting level of this macro
	d->nesting_ = nesting;

	// update requires
	d->required_ = d->macro_->required();

	if (!d->needsUpdate_
		// non-normal mode? We are done!
		|| (d->displayMode_ != DISPLAY_NORMAL))
		return;

	d->needsUpdate_ = false;

	// get default values of macro
	vector<docstring> const & defaults = d->macro_->defaults();

	// create MathMacroArgumentValue objects pointing to the cells of the macro
	vector<MathData> values(nargs());
	for (size_t i = 0; i < nargs(); ++i) {
		InsetArgumentProxy * proxy;
		if (i < defaults.size())
			proxy = new InsetArgumentProxy(this, i, defaults[i]);
		else
			proxy = new InsetArgumentProxy(this, i);
		values[i].insert(0, MathAtom(proxy));
	}
	// expanding macro with the values
	// Only update the argument macros if anything was expanded or the LyX
	// representation part does not contain the macro itself, otherwise we
	// would get an endless loop (bugs 9140 and 11595). UpdateLocker does
	// not work in this case, since MacroData::expand() creates new
	// InsetMathMacro objects, so this would be a different recursion path
	// than the one protected by UpdateLocker.
	docstring const & display = d->macro_->display();
	docstring const latexname = from_ascii("\\") + macroName();
	bool const ret = d->macro_->expand(values, d->expanded_);
	d->expanded_.setBuffer(buffer());
	if (ret && !support::contains(display, latexname)) {
		if (utype == OutputUpdate && !d->expanded_.empty())
			d->expanded_.updateMacros(cur, mc, utype, nesting);
	}
	// get definition for list edit mode
	asArray(display.empty() ? d->macro_->definition() : display,
		d->definition_, Parse::QUIET | Parse::MACRODEF);
}


void InsetMathMacro::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);

	int expx = x;
	int expy = y;

	if (d->displayMode_ == DISPLAY_INIT || d->displayMode_ == DISPLAY_INTERACTIVE_INIT) {
		Changer dummy = pi.base.changeFontSet("lyxtex");
		pi.pain.text(x, y, from_ascii("\\") + name(), pi.base.font);
	} else if (d->displayMode_ == DISPLAY_UNFOLDED) {
		Changer dummy = pi.base.changeFontSet("lyxtex");
		pi.pain.text(x, y, from_ascii("\\"), pi.base.font);
		x += mathed_string_width(pi.base.font, from_ascii("\\")) + 1;
		cell(0).draw(pi, x, y);
	} else if (lyxrc.macro_edit_style == LyXRC::MACRO_EDIT_LIST
	           && d->editing_[pi.base.bv]) {
		// Macro will be edited in a old-style list mode here:

		CoordCache const & coords = pi.base.bv->coordCache();
		FontInfo const & labelFont = sane_font;

		// box needs one pixel
		x += 1;

		// get maximal font height
		Dimension fontDim;
		math_font_max_dim(pi.base.font, fontDim.asc, fontDim.des);

		// draw label
		docstring label = from_ascii("Macro \\") + name() + from_ascii(": ");
		pi.pain.text(x, y, label, labelFont);
		x += mathed_string_width(labelFont, label);

		// draw definition
		d->definition_.draw(pi, x, y);
		Dimension const & defDim = coords.getArrays().dim(&d->definition_);
		y += max(fontDim.des, defDim.des);

		// draw parameters
		docstring str = from_ascii("#9");
		int strw1 = mathed_string_width(labelFont, from_ascii("#9"));
		int strw2 = mathed_string_width(labelFont, from_ascii(": "));

		for (idx_type i = 0; i < nargs(); ++i) {
			// position of label
			Dimension const & cdim = coords.getArrays().dim(&cell(i));
			x = expx + 1;
			y += max(fontDim.asc, cdim.asc) + 1;

			// draw label
			str[1] = '1' + i;
			pi.pain.text(x, y, str, labelFont);
			x += strw1;
			pi.pain.text(x, y, from_ascii(":"), labelFont);
			x += strw2;

			// draw parameter
			cell(i).draw(pi, x, y);

			// next line
			y += max(fontDim.des, cdim.des);
		}

		pi.pain.rectangle(expx, expy - dim.asc + 1, dim.wid - 1,
				  dim.height() - 2, Color_mathmacroframe);
	} else {
		// We should not be here, since the macro is linearized in this case.
		LBUFERR(false);
	}

	// edit mode changed?
	if (d->editing_[pi.base.bv] != editMode(pi.base.bv))
		pi.base.bv->cursor().screenUpdateFlags(Update::SinglePar);
}


void InsetMathMacro::setDisplayMode(InsetMathMacro::DisplayMode mode, int appetite)
{
	if (d->displayMode_ != mode) {
		// transfer name if changing from or to DISPLAY_UNFOLDED
		if (mode == DISPLAY_UNFOLDED) {
			cells_.resize(1);
			asArray(d->name_, cell(0));
		} else if (d->displayMode_ == DISPLAY_UNFOLDED) {
			d->name_ = asString(cell(0));
			cells_.resize(0);
		}

		d->displayMode_ = mode;
		d->needsUpdate_ = true;
	}

	// the interactive init mode is non-greedy by default
	if (appetite == -1)
		d->appetite_ = (mode == DISPLAY_INTERACTIVE_INIT) ? 0 : 9;
	else
		d->appetite_ = size_t(appetite);
}


InsetMathMacro::DisplayMode InsetMathMacro::computeDisplayMode() const
{
	if (d->nextFoldMode_ && d->macro_ && !d->macro_->locked())
		return DISPLAY_NORMAL;
	else
		return DISPLAY_UNFOLDED;
}


bool InsetMathMacro::validName() const
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
	if (n.size() > 1) {
		for (char_type c : n) {
			if (!(c >= 'a' && c <= 'z')
			    && !(c >= 'A' && c <= 'Z')
			    && c != '*')
				return false;
		}
	}

	return true;
}


size_t InsetMathMacro::arity() const
{
	if (d->displayMode_ == DISPLAY_NORMAL )
		return cells_.size();
	else
		return 0;
}


size_t InsetMathMacro::optionals() const
{
	return d->optionals_;
}


void InsetMathMacro::setOptionals(int n)
{
	if (n <= int(nargs()))
		d->optionals_ = n;
}


size_t InsetMathMacro::appetite() const
{
	return d->appetite_;
}


MathClass InsetMathMacro::mathClass() const
{
	// This can be just a heuristic, since it is only considered for display
	// when the macro is not linearised. Therefore it affects:
	// * The spacing of the inset while being edited,
	// * Intelligent splitting
	// * Cursor word movement (Ctrl-Arrow).
	if (MacroData const * m = macroBackup()) {
		// If it is a global macro and is defined explicitly
		if (m->symbol()) {
			MathClass mc = string_to_class(m->symbol()->extra);
			if (mc != MC_UNKNOWN)
				return mc;
		}
	}
	// Otherwise guess from the expanded macro
	return d->expanded_.mathClass();
}


InsetMath::mode_type InsetMathMacro::currentMode() const
{
	// User defined macros are always assumed to be mathmode macros.
	// Only the global macros defined in lib/symbols may be textmode.
	if (MacroData const * m = macroBackup()) {
		if (m->symbol() && m->symbol()->extra == "textmode")
			return TEXT_MODE;
		else
			return MATH_MODE;
	}
	// Unknown macros are undecided.
	return UNDECIDED_MODE;
}


MacroData const * InsetMathMacro::macroBackup() const
{
	if (macro())
		return &d->macroBackup_;
	if (MacroData const * data = MacroTable::globalMacros().get(name()))
		return data;
	return nullptr;
}


void InsetMathMacro::validate(LaTeXFeatures & features) const
{
	// Immediately after a document is loaded, in some cases the MacroData
	// of the global macros defined in the lib/symbols file may still not
	// be known to the macro machinery because it will be set only after
	// the first call to updateMacros(). This is not a problem unless
	// instant preview is on for math, in which case we will be missing
	// the corresponding requirements.
	// In this case, we get the required info from the global macro table.
	if (!d->required_.empty())
		features.require(d->required_);
	else if (!d->macro_) {
		// Update requires for known global macros.
		MacroData const * data = MacroTable::globalMacros().get(name());
		if (data && !data->required().empty())
			features.require(data->required());
	}

	if (name() == "binom")
		features.require("binom");

	// validate the cells and the definition
	if (displayMode() == DISPLAY_NORMAL) {
		// Don't update requirements if the macro comes from
		// the symbols file and has not been redefined.
		MathWordList const & words = mathedWordList();
		MathWordList::const_iterator it = words.find(name());
		MacroNameSet macros;
		buffer().listMacroNames(macros);
		if (it == words.end() || it->second.inset != "macro"
		    || macros.find(name()) != macros.end()) {
			d->definition_.validate(features);
		}
		InsetMathNest::validate(features);
	}
}


void InsetMathMacro::edit(Cursor & cur, bool front, EntryDirection entry_from)
{
	cur.screenUpdateFlags(Update::SinglePar);
	InsetMathNest::edit(cur, front, entry_from);
}


Inset * InsetMathMacro::editXY(Cursor & cur, int x, int y)
{
	// We may have 0 arguments, but InsetMathNest requires at least one.
	if (nargs() > 0) {
		cur.screenUpdateFlags(Update::SinglePar);
		return InsetMathNest::editXY(cur, x, y);
	} else
		return this;
}


void InsetMathMacro::removeArgument(pos_type pos) {
	if (d->displayMode_ == DISPLAY_NORMAL) {
		LASSERT(size_t(pos) < cells_.size(), return);
		cells_.erase(cells_.begin() + pos);
		if (size_t(pos) < d->attachedArgsNum_)
			--d->attachedArgsNum_;
		if (size_t(pos) < d->optionals_) {
			--d->optionals_;
		}

		d->needsUpdate_ = true;
	}
}


void InsetMathMacro::insertArgument(pos_type pos) {
	if (d->displayMode_ == DISPLAY_NORMAL) {
		LASSERT(size_t(pos) <= cells_.size(), return);
		cells_.insert(cells_.begin() + pos, MathData());
		if (size_t(pos) < d->attachedArgsNum_)
			++d->attachedArgsNum_;
		if (size_t(pos) < d->optionals_)
			++d->optionals_;

		d->needsUpdate_ = true;
	}
}


void InsetMathMacro::detachArguments(vector<MathData> & args, bool strip)
{
	LASSERT(d->displayMode_ == DISPLAY_NORMAL, return);
	args = cells_;

	// strip off empty cells, but not more than arity-attachedArgsNum_
	if (strip) {
		size_t i;
		for (i = cells_.size(); i > d->attachedArgsNum_; --i)
			if (!cell(i - 1).empty()) break;
		args.resize(i);
	}

	d->attachedArgsNum_ = 0;
	d->expanded_ = MathData();
	cells_.resize(0);

	d->needsUpdate_ = true;
}


void InsetMathMacro::attachArguments(vector<MathData> const & args, size_t arity, int optionals)
{
	LASSERT(d->displayMode_ == DISPLAY_NORMAL, return);
	cells_ = args;
	d->attachedArgsNum_ = args.size();
	cells_.resize(arity);
	d->expanded_ = MathData();
	d->optionals_ = optionals;

	d->needsUpdate_ = true;
}


bool InsetMathMacro::idxFirst(Cursor & cur) const
{
	cur.screenUpdateFlags(Update::SinglePar);
	return InsetMathNest::idxFirst(cur);
}


bool InsetMathMacro::idxLast(Cursor & cur) const
{
	cur.screenUpdateFlags(Update::SinglePar);
	return InsetMathNest::idxLast(cur);
}


bool InsetMathMacro::notifyCursorLeaves(Cursor const & old, Cursor & cur)
{
	if (d->displayMode_ == DISPLAY_UNFOLDED) {
		docstring const & unfolded_name = name();
		if (unfolded_name != d->name_) {
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


void InsetMathMacro::fold(Cursor & cur)
{
	if (!d->nextFoldMode_) {
		d->nextFoldMode_ = true;
		cur.screenUpdateFlags(Update::SinglePar);
	}
}


void InsetMathMacro::unfold(Cursor & cur)
{
	if (d->nextFoldMode_) {
		d->nextFoldMode_ = false;
		cur.screenUpdateFlags(Update::SinglePar);
	}
}


bool InsetMathMacro::folded() const
{
	return d->nextFoldMode_;
}


void InsetMathMacro::write(TeXMathStream & os) const
{
	mode_type mode = currentMode();
	MathEnsurer ensurer(os, mode == MATH_MODE, true, mode == TEXT_MODE);

	// non-normal mode
	if (d->displayMode_ != DISPLAY_NORMAL) {
		os << "\\" << name();
		if (name().size() != 1 || isAlphaASCII(name()[0]))
			os.pendingSpace(true);
		return;
	}

	// normal mode
	// we should be ok to continue even if this fails.
	LATTEST(d->macro_);

	// Always protect macros in a fragile environment
	if (os.fragile())
		os << "\\protect";

	os << "\\" << name();
	bool first = true;

	// Optional arguments:
	// First find last non-empty optional argument
	idx_type emptyOptFrom = 0;
	idx_type i = 0;
	for (; i < cells_.size() && i < d->optionals_; ++i) {
		if (!cell(i).empty())
			emptyOptFrom = i + 1;
	}

	// print out optionals
	for (i=0; i < cells_.size() && i < emptyOptFrom; ++i) {
		first = false;
		// For correctly parsing it when a document is reloaded, we
		// need to enclose an optional argument in braces if it starts
		// with a script inset with empty nucleus or ends with a
		// delimiter-size-modifier macro (see #10497 and #11346).
		// We also need to do that when the optional argument
		// contains macros with optionals.
		bool braced = false;
		size_type last = cell(i).size() - 1;
		if (!cell(i).empty() && cell(i)[last]->asUnknownInset()) {
			latexkeys const * l = in_word_set(cell(i)[last]->name());
			braced = (l && l->inset == "big");
		} else if (!cell(i).empty() && cell(i)[0]->asScriptInset()) {
			braced = cell(i)[0]->asScriptInset()->nuc().empty();
		} else {
			for (size_type j = 0; j < cell(i).size(); ++j) {
				InsetMathMacro const * ma = cell(i)[j]->asMacro();
				if (ma && ma->optionals()) {
					braced = true;
					break;
				}
			}
		}
		if (braced)
			os << "[{" << cell(i) << "}]";
		else
			os << "[" << cell(i) << "]";
	}

	// skip the tailing empty optionals
	i = d->optionals_;

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

	// write \(no)limits modifiers if relevant
	writeLimits(os);
}


void InsetMathMacro::maple(MapleStream & os) const
{
	lyx::maple(d->expanded_, os);
}


void InsetMathMacro::maxima(MaximaStream & os) const
{
	lyx::maxima(d->expanded_, os);
}


void InsetMathMacro::mathematica(MathematicaStream & os) const
{
	lyx::mathematica(d->expanded_, os);
}


void InsetMathMacro::mathmlize(MathMLStream & ms) const
{
	// macro_ is 0 if this is an unknown macro
	LATTEST(d->macro_ || d->displayMode_ != DISPLAY_NORMAL);
	if (d->macro_) {
		docstring const xmlname = (ms.xmlMode()) ? d->macro_->xmlname() : d->macro_->htmlname();
		if (!xmlname.empty()) {
			char const * type = d->macro_->MathMLtype();
			ms << "<" << from_ascii(ms.namespacedTag(type)) << ">"
			   << xmlname
			   << "</" << from_ascii(ms.namespacedTag(type)) << ">";
			return;
		}
	}
	if (d->expanded_.empty()) {
		// this means that we do not recognize the macro
		throw MathExportException();
	}
	ms << d->expanded_;
}


void InsetMathMacro::htmlize(HtmlStream & os) const
{
	// macro_ is 0 if this is an unknown macro
	LATTEST(d->macro_ || d->displayMode_ != DISPLAY_NORMAL);
	if (d->macro_) {
		docstring const xmlname = d->macro_->htmlname();
		if (!xmlname.empty()) {
			os << ' ' << xmlname << ' ';
			return;
		}
	}
	if (d->expanded_.empty()) {
		// this means that we do not recognize the macro
		throw MathExportException();
	}
	os << d->expanded_;
}


void InsetMathMacro::octave(OctaveStream & os) const
{
	lyx::octave(d->expanded_, os);
}


void InsetMathMacro::infoize(odocstream & os) const
{
	os << bformat(_("Macro: %1$s"), name());
}


void InsetMathMacro::infoize2(odocstream & os) const
{
	os << bformat(_("Macro: %1$s"), name());
}


bool InsetMathMacro::completionSupported(Cursor const & cur) const
{
	if (displayMode() != DISPLAY_UNFOLDED)
		return InsetMathNest::completionSupported(cur);

	return lyxrc.completion_popup_math
		&& cur.bv().cursor().pos() == int(name().size());
}


bool InsetMathMacro::inlineCompletionSupported(Cursor const & cur) const
{
	if (displayMode() != DISPLAY_UNFOLDED)
		return InsetMathNest::inlineCompletionSupported(cur);

	return lyxrc.completion_inline_math
		&& cur.bv().cursor().pos() == int(name().size());
}


bool InsetMathMacro::automaticInlineCompletion() const
{
	if (displayMode() != DISPLAY_UNFOLDED)
		return InsetMathNest::automaticInlineCompletion();

	return lyxrc.completion_inline_math;
}


bool InsetMathMacro::automaticPopupCompletion() const
{
	if (displayMode() != DISPLAY_UNFOLDED)
		return InsetMathNest::automaticPopupCompletion();

	return lyxrc.completion_popup_math;
}


CompletionList const *
InsetMathMacro::createCompletionList(Cursor const & cur) const
{
	if (displayMode() != DISPLAY_UNFOLDED)
		return InsetMathNest::createCompletionList(cur);

	return new MathCompletionList(cur.bv().cursor());
}


docstring InsetMathMacro::completionPrefix(Cursor const & cur) const
{
	if (displayMode() != DISPLAY_UNFOLDED)
		return InsetMathNest::completionPrefix(cur);

	if (!completionSupported(cur))
		return docstring();

	return "\\" + name();
}


bool InsetMathMacro::insertCompletion(Cursor & cur, docstring const & s,
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


void InsetMathMacro::completionPosAndDim(Cursor const & cur, int & x, int & y,
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


void InsetMathMacro::setBuffer(Buffer & buffer)
{
	d->definition_.setBuffer(buffer);
	InsetMathNest::setBuffer(buffer);
}

} // namespace lyx
