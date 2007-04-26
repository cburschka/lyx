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
#include "LCursor.h"
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
class MathMacroArgumentValue : public InsetMathDim {
public:
	///
	MathMacroArgumentValue(MathArray const * value, docstring const & macroName) 
		: value_(value), macroName_(macroName) {}
	///
	bool metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	
private:
	std::auto_ptr<InsetBase> doClone() const;
	MathArray const * value_;
	docstring macroName_;
};


auto_ptr<InsetBase> MathMacroArgumentValue::doClone() const 
{
	return auto_ptr<InsetBase>(new MathMacroArgumentValue(*this));
}


bool MathMacroArgumentValue::metrics(MetricsInfo & mi, Dimension & dim) const 
{
	// unlock outer macro in arguments, and lock it again later
	MacroTable::globalMacros().get(macroName_).unlock();
	value_->metrics(mi, dim);
	MacroTable::globalMacros().get(macroName_).lock();
	metricsMarkers2(dim);
	if (dim_ == dim)
		return false;
	dim_ = dim;
	return true;
}


void MathMacroArgumentValue::draw(PainterInfo & pi, int x, int y) const 
{
	// unlock outer macro in arguments, and lock it again later
	MacroTable::globalMacros().get(macroName_).unlock();
	value_->draw(pi, x, y);
	MacroTable::globalMacros().get(macroName_).lock();
}


MathMacro::MathMacro(docstring const & name, int numargs)
	: InsetMathNest(numargs), name_(name)
{}


auto_ptr<InsetBase> MathMacro::doClone() const
{
	return auto_ptr<InsetBase>(new MathMacro(*this));
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
	if (!MacroTable::globalMacros().has(name())) {
		mathed_string_dim(mi.base.font, "Unknown: " + name(), dim);
	} else {
		MacroData const & macro = MacroTable::globalMacros().get(name());
		if (macro.locked()) {
			mathed_string_dim(mi.base.font, "Self reference: " + name(), dim);
			expanded_ = MathArray();
		} else if (editing(mi.base.bv)) {
			// FIXME UNICODE
			asArray(macro.def(), tmpl_);
			LyXFont font = mi.base.font;
			augmentFont(font, from_ascii("lyxtex"));
			tmpl_.metrics(mi, dim);
			// FIXME UNICODE
			dim.wid += mathed_string_width(font, name()) + 10;
			// FIXME UNICODE
			int ww = mathed_string_width(font, from_ascii("#1: "));
			for (idx_type i = 0; i < nargs(); ++i) {
				MathArray const & c = cell(i);
				c.metrics(mi);
				dim.wid  = max(dim.wid, c.width() + ww);
				dim.des += c.height() + 10;
			}
		} else {
			// create MathMacroArgumentValue object pointing to the cells of the macro
			MacroData const & macro = MacroTable::globalMacros().get(name());
			vector<MathArray> values(nargs());
			for (size_t i = 0; i != nargs(); ++i) 
				values[i].insert(0, MathAtom(new MathMacroArgumentValue(&cells_[i], name())));
			macro.expand(values, expanded_);
			
			MacroTable::globalMacros().get(name()).lock();
			expanded_.metrics(mi, dim);
			MacroTable::globalMacros().get(name()).unlock();
		}
	}
	metricsMarkers2(dim);
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
		if (macro.locked()) {
			// FIXME UNICODE
			drawStrRed(pi, x, y, "Self reference: " + name());
		} else if (editing(pi.base.bv)) {
			LyXFont font = pi.base.font;
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
				MathArray const & c = cell(i);
				h += max(c.ascent(), ldim.asc) + 5;
				c.draw(pi, x + ldim.wid, h);
				char_type str[] = { '#', '1', ':', '\0' };
				str[1] += static_cast<char_type>(i);
				pi.pain.text(x + 3, h, str, font);
				h += max(c.descent(), ldim.des) + 5;
			}
		} else {
			MacroTable::globalMacros().get(name()).lock();
			expanded_.draw(pi, x, y);
			MacroTable::globalMacros().get(name()).unlock();
		}
	}
	drawMarkers2(pi, x, y);
}


void MathMacro::drawSelection(PainterInfo & pi, int x, int y) const
{
	// We may have 0 arguments, but InsetMathNest requires at least one.
	if (nargs() > 0)
		InsetMathNest::drawSelection(pi, x, y);
}


void MathMacro::validate(LaTeXFeatures & features) const
{
	if (name() == "binom" || name() == "mathcircumflex")
		features.require(to_utf8(name()));
}


InsetBase * MathMacro::editXY(LCursor & cur, int x, int y)
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


bool MathMacro::idxFirst(LCursor & cur) const 
{
	cur.updateFlags(Update::Force);
	return InsetMathNest::idxFirst(cur);
}


bool MathMacro::idxLast(LCursor & cur) const 
{
	cur.updateFlags(Update::Force);
	return InsetMathNest::idxLast(cur);
}


bool MathMacro::notifyCursorLeaves(LCursor & cur)
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
	//expanded_.substitute(*this);
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
