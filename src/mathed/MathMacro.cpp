/**
 * \file MathMacro.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MathMacro.h"
#include "MathSupport.h"
#include "MathExtern.h"
#include "MathStream.h"

#include "Buffer.h"
#include "Cursor.h"
#include "debug.h"
#include "BufferView.h"
#include "LaTeXFeatures.h"
#include "frontends/Painter.h"


namespace lyx {

using std::string;
using std::max;
using std::auto_ptr;
using std::endl;
using std::vector;


/// This class is the value of a macro argument, technically
/// a wrapper of the cells of MathMacro.
class MathMacroArgumentValue : public InsetMath {
public:
	///
	MathMacroArgumentValue(MathMacro const & mathMacro, size_t idx)
		: mathMacro_(mathMacro), idx_(idx) {}
	///
	bool metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	int kerning() const { return mathMacro_.cell(idx_).kerning(); }

private:
	std::auto_ptr<Inset> doClone() const;
	MathMacro const & mathMacro_;
	size_t idx_;
};


auto_ptr<Inset> MathMacroArgumentValue::doClone() const
{
	return auto_ptr<Inset>(new MathMacroArgumentValue(*this));
}


bool MathMacroArgumentValue::metrics(MetricsInfo & mi, Dimension & dim) const
{
	// unlock outer macro in arguments, and lock it again later
	MacroData const & macro = MacroTable::globalMacros().get(mathMacro_.name());
	macro.unlock();
	mathMacro_.cell(idx_).metrics(mi, dim);
	macro.lock();
	if (dim_ == dim)
		return false;
	dim_ = dim;
	return true;
}


void MathMacroArgumentValue::draw(PainterInfo & pi, int x, int y) const
{
	// unlock outer macro in arguments, and lock it again later
	MacroData const & macro = MacroTable::globalMacros().get(mathMacro_.name());
	macro.unlock();
	mathMacro_.cell(idx_).draw(pi, x, y);
	macro.lock();
}


MathMacro::MathMacro(docstring const & name, int numargs)
	: InsetMathNest(numargs), name_(name), editing_(false)
{}


auto_ptr<Inset> MathMacro::doClone() const
{
	MathMacro * x = new MathMacro(*this);
	x->expanded_ = MathData();
	x->macroBackup_ = MacroData();
	return auto_ptr<Inset>(x);
}


docstring MathMacro::name() const
{
	return name_;
}


void MathMacro::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x,	int & y) const
{
	// We may have 0 arguments, but InsetMathNest requires at least one.
	if (nargs() > 0)
		InsetMathNest::cursorPos(bv, sl, boundary, x, y);
}


bool MathMacro::metrics(MetricsInfo & mi, Dimension & dim) const
{
	kerning_ = 0;
	if (!MacroTable::globalMacros().has(name())) {
		mathed_string_dim(mi.base.font, "Unknown: " + name(), dim);
	} else {
		MacroData const & macro = MacroTable::globalMacros().get(name());

		if (macroBackup_ != macro)
			updateExpansion();

		if (macro.locked()) {
			mathed_string_dim(mi.base.font, "Self reference: " + name(), dim);
		} else if (editing(mi.base.bv)) {
			Font font = mi.base.font;
			augmentFont(font, from_ascii("lyxtex"));
			tmpl_.metrics(mi, dim);
			// FIXME UNICODE
			dim.wid += mathed_string_width(font, name()) + 10;
			// FIXME UNICODE
			int ww = mathed_string_width(font, from_ascii("#1: "));
			for (idx_type i = 0; i < nargs(); ++i) {
				MathData const & c = cell(i);
				c.metrics(mi);
				dim.wid  = max(dim.wid, c.width() + ww);
				dim.des += c.height() + 10;
			}
			editing_ = true;
		} else {
			macro.lock();
			expanded_.metrics(mi, dim);
			macro.unlock();
			kerning_ = expanded_.kerning();
			editing_ = false;
		}
	}
	if (dim_ == dim)
		return false;
	dim_ = dim;
	return true;
}


void MathMacro::draw(PainterInfo & pi, int x, int y) const
{
	if (!MacroTable::globalMacros().has(name())) {
		// FIXME UNICODE
		drawStrRed(pi, x, y, "Unknown: " + name());
	} else {
		MacroData const & macro = MacroTable::globalMacros().get(name());

		// warm up cache
		for (size_t i = 0; i < nargs(); ++i)
			cell(i).setXY(*pi.base.bv, x, y);

		if (macro.locked()) {
			// FIXME UNICODE
			drawStrRed(pi, x, y, "Self reference: " + name());
		} else if (editing_) {
			Font font = pi.base.font;
			augmentFont(font, from_ascii("lyxtex"));
			int h = y - dim_.ascent() + 2 + tmpl_.ascent();
			pi.pain.text(x + 3, h, name(), font);
			int const w = mathed_string_width(font, name());
			tmpl_.draw(pi, x + w + 12, h);
			h += tmpl_.descent();
			Dimension ldim;
			docstring t = from_ascii("#1: ");
			mathed_string_dim(font, t, ldim);
			for (idx_type i = 0; i < nargs(); ++i) {
				MathData const & c = cell(i);
				h += max(c.ascent(), ldim.asc) + 5;
				c.draw(pi, x + ldim.wid, h);
				char_type str[] = { '#', '1', ':', '\0' };
				str[1] += static_cast<char_type>(i);
				pi.pain.text(x + 3, h, str, font);
				h += max(c.descent(), ldim.des) + 5;
			}
		} else {
			macro.lock();
			expanded_.draw(pi, x, y);
			macro.unlock();
		}

		// edit mode changed?
		if (editing_ != editing(pi.base.bv) || macroBackup_ != macro)
			pi.base.bv->cursor().updateFlags(Update::Force);
	}
}


void MathMacro::drawSelection(PainterInfo & pi, int x, int y) const
{
	// We may have 0 arguments, but InsetMathNest requires at least one.
	if (nargs() > 0)
		InsetMathNest::drawSelection(pi, x, y);
}


void MathMacro::validate(LaTeXFeatures & features) const
{
	string const require = MacroTable::globalMacros().get(name()).requires();
	if (!require.empty())
		features.require(require);

	if (name() == "binom" || name() == "mathcircumflex")
		features.require(to_utf8(name()));
}


Inset * MathMacro::editXY(Cursor & cur, int x, int y)
{
	// We may have 0 arguments, but InsetMathNest requires at least one.
	if (nargs() > 0) {
		// Prevent crash due to cold coordcache
		// FIXME: This is only a workaround, the call of
		// InsetMathNest::editXY is correct. The correct fix would
		// ensure that the coordcache of the arguments is valid.
		if (!editing(&cur.bv())) {
			edit(cur, true);
			return this;
		}
		return InsetMathNest::editXY(cur, x, y);
	}
	return this;
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


bool MathMacro::idxUpDown(Cursor & cur, bool up) const
{
	if (up) {
		if (cur.idx() == 0)
			return false;
		--cur.idx();
	} else {
		if (cur.idx() + 1 >= nargs())
			return false;
		++cur.idx();
	}
	cur.pos() = cell(cur.idx()).x2pos(cur.x_target());
	return true;
}


bool MathMacro::notifyCursorLeaves(Cursor & cur)
{
	cur.updateFlags(Update::Force);
	return InsetMathNest::notifyCursorLeaves(cur);
}


void MathMacro::maple(MapleStream & os) const
{
	updateExpansion();
	lyx::maple(expanded_, os);
}


void MathMacro::mathmlize(MathStream & os) const
{
	updateExpansion();
	lyx::mathmlize(expanded_, os);
}


void MathMacro::octave(OctaveStream & os) const
{
	updateExpansion();
	lyx::octave(expanded_, os);
}


void MathMacro::updateExpansion() const
{
	MacroData const & macro = MacroTable::globalMacros().get(name());

	// create MathMacroArgumentValue object pointing to the cells of the macro
	vector<MathData> values(nargs());
	for (size_t i = 0; i != nargs(); ++i)
				values[i].insert(0, MathAtom(new MathMacroArgumentValue(*this, i)));
	macro.expand(values, expanded_);
	asArray(macro.def(), tmpl_);
	macroBackup_ = macro;
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
