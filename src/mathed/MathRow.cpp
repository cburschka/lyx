/**
 * \file MathRow.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MathRow.h"

#include "InsetMath.h"
#include "MathClass.h"
#include "MathData.h"
#include "MathMacro.h"
#include "MathSupport.h"

#include "BufferView.h"
#include "CoordCache.h"
#include "MetricsInfo.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/lassert.h"

#include <ostream>

using namespace std;

namespace lyx {


MathRow::Element::Element(Type t, MetricsInfo &mi)
	: type(t), macro_nesting(mi.base.macro_nesting),
	  inset(0), mclass(MC_ORD), before(0), after(0), compl_unique_to(0),
	  macro(0), color(Color_red)
{}


MathRow::MathRow(MetricsInfo & mi, MathData const * ar)
{
	// First there is a dummy element of type "open"
	push_back(Element(BEGIN, mi));
	back().mclass = MC_OPEN;

	// Then insert the MathData argument
	bool const has_contents = ar->addToMathRow(*this, mi);

	// empty arrays are visible when they are editable
	// we reserve the necessary space anyway (even if nothing gets drawn)
	if (!has_contents) {
		Element e(BOX, mi);
		e.color = Color_mathline;
		push_back(e);
	}

	// Finally there is a dummy element of type "close"
	push_back(Element(END, mi));
	back().mclass = MC_CLOSE;

	/* Do spacing only in math mode. This test is a bit clumsy,
	 * but it is used in other places for guessing the current mode.
	 */
	if (!isMathFont(mi.base.fontname))
		return;

	// update classes
	for (int i = 1 ; i != static_cast<int>(elements_.size()) - 1 ; ++i) {
		if (elements_[i].type != INSET)
			continue;
		update_class(elements_[i].mclass, elements_[before(i)].mclass,
		             elements_[after(i)].mclass);
	}

	// set spacing
	// We go to the end to handle spacing at the end of equation
	for (int i = 1 ; i != static_cast<int>(elements_.size()) ; ++i) {
		if (elements_[i].type != INSET)
			continue;
		Element & bef = elements_[before(i)];
		int spc = class_spacing(bef.mclass, elements_[i].mclass, mi.base);
		bef.after = spc / 2;
		// this is better than spc / 2 to avoid rounding problems
		elements_[i].before = spc - spc / 2;
	}
	// Do not lose spacing allocated to extremities
	if (!elements_.empty()) {
		elements_[after(0)].before += elements_.front().after;
		elements_[before(elements_.size() - 1)].after += elements_.back().before;
	}
}


int MathRow::before(int i) const
{
	do
		--i;
	while (elements_[i].type != BEGIN
		   && elements_[i].type != INSET);

	return i;
}


int MathRow::after(int i) const
{
	do
		++i;
	while (elements_[i].type != END
		   && elements_[i].type != INSET);

	return i;
}


void MathRow::metrics(MetricsInfo & mi, Dimension & dim) const
{
	dim.asc = 0;
	dim.wid = 0;
	// In order to compute the dimension of macros and their
	// arguments, it is necessary to keep track of them.
	map<MathMacro const *, Dimension> dim_macros;
	map<MathData const *, Dimension> dim_arrays;
	CoordCache & coords = mi.base.bv->coordCache();
	for (Element const & e : elements_) {
		Dimension d;
		mi.base.macro_nesting = e.macro_nesting;
		switch (e.type) {
		case BEGIN:
		case END:
			break;
		case INSET:
			e.inset->metrics(mi, d);
			d.wid += e.before + e.after;
			coords.insets().add(e.inset, d);
			break;
		case BEG_MACRO:
			e.macro->macro()->lock();
			// Add a macro to current list
			dim_macros[e.macro] = Dimension();
			break;
		case END_MACRO:
			LATTEST(dim_macros.find(e.macro) != dim_macros.end());
			e.macro->macro()->unlock();
			// Cache the dimension of the macro and remove it from
			// tracking map.
			coords.insets().add(e.macro, dim_macros[e.macro]);
			dim_macros.erase(e.macro);
			break;
			// This is basically like macros
		case BEG_ARG:
			if (e.macro)
				e.macro->macro()->unlock();
			dim_arrays[e.ar] = Dimension();
			break;
		case END_ARG:
			LATTEST(dim_arrays.find(e.ar) != dim_arrays.end());
			if (e.macro)
				e.macro->macro()->lock();
			coords.arrays().add(e.ar, dim_arrays[e.ar]);
			dim_arrays.erase(e.ar);
			break;
		case BOX:
			d = theFontMetrics(mi.base.font).dimension('I');
			d.wid += e.before + e.after;
			break;
		}

		if (!d.empty()) {
			dim += d;
			// Now add the dimension to current macros and arguments.
			for (auto & dim_macro : dim_macros)
				dim_macro.second += d;
			for (auto & dim_array : dim_arrays)
				dim_array.second += d;
		}

		if (e.compl_text.empty())
			continue;
		FontInfo font = mi.base.font;
		augmentFont(font, "mathnormal");
		dim.wid += mathed_string_width(font, e.compl_text);
	}
	LATTEST(dim_macros.empty() && dim_arrays.empty());
}


void MathRow::draw(PainterInfo & pi, int x, int const y) const
{
	CoordCache & coords = pi.base.bv->coordCache();
	for (Element const & e : elements_) {
		pi.base.macro_nesting = e.macro_nesting;
		switch (e.type) {
		case INSET: {
			// This is hackish: the math inset does not know that space
			// has been added before and after it; we alter its dimension
			// while it is drawing, because it relies on this value.
			Dimension const d = coords.insets().dim(e.inset);
			Dimension d2 = d;
			d2.wid -= e.before + e.after;
			coords.insets().add(e.inset, d2);
			e.inset->drawSelection(pi, x + e.before, y);
			e.inset->draw(pi, x + e.before, y);
			coords.insets().add(e.inset, x, y);
			coords.insets().add(e.inset, d);
			x += d.wid;
			break;
		}
		case BEG_MACRO:
			coords.insets().add(e.macro, x, y);
			break;
		case BEG_ARG:
			coords.arrays().add(e.ar, x, y);
			// if the macro is being edited, then the painter is in
			// monochrome mode.
			if (e.macro->editMetrics(pi.base.bv))
				pi.pain.leaveMonochromeMode();
			break;
		case END_ARG:
			if (e.macro->editMetrics(pi.base.bv))
				pi.pain.enterMonochromeMode(Color_mathbg, Color_mathmacroblend);
			break;
		case BOX: {
			Dimension const d = theFontMetrics(pi.base.font).dimension('I');
			// the box is not visible in non-editable context (except for grey macro boxes).
			if (e.macro_nesting == 0 || e.color == Color_mathmacroblend)
				pi.pain.rectangle(x + e.before, y - d.ascent(),
								  d.width(), d.height(), e.color);
			x += d.wid;
			break;
		}
		case BEGIN:
		case END:
		case END_MACRO:
			break;
		}

		if (e.compl_text.empty())
			continue;
		FontInfo f = pi.base.font;
		augmentFont(f, "mathnormal");

		// draw the unique and the non-unique completion part
		// Note: this is not time-critical as it is
		// only done once per screen.
		docstring const s1 = e.compl_text.substr(0, e.compl_unique_to);
		docstring const s2 = e.compl_text.substr(e.compl_unique_to);

		if (!s1.empty()) {
			f.setColor(Color_inlinecompletion);
			pi.pain.text(x, y, s1, f);
			x += mathed_string_width(f, s1);
		}
		if (!s2.empty()) {
			f.setColor(Color_nonunique_inlinecompletion);
			pi.pain.text(x, y, s2, f);
			x += mathed_string_width(f, s2);
		}
	}
}


int MathRow::kerning(BufferView const * bv) const
{
	if (elements_.empty())
		return 0;
	InsetMath const * inset = elements_[before(elements_.size() - 1)].inset;
	return inset ? inset->kerning(bv) : 0;
}


ostream & operator<<(ostream & os, MathRow::Element const & e)
{
	switch (e.type) {
	case MathRow::BEGIN:
		os << "{";
		break;
	case MathRow::END:
		os << "}";
		break;
	case MathRow::INSET:
		os << "<" << e.before << "-"
		   << to_utf8(class_to_string(e.mclass))
		   << "-" << e.after << ">";
		break;
	case MathRow::BEG_MACRO:
		os << "\\" << to_utf8(e.macro->name()) << "[";
		break;
	case MathRow::END_MACRO:
		os << "]";
		break;
	case MathRow::BEG_ARG:
		os << "#(";
		break;
	case MathRow::END_ARG:
		os << ")";
		break;
	case MathRow::BOX:
		os << "@";
		break;
	}
	return os;
}


ostream & operator<<(ostream & os, MathRow const & mrow)
{
	for (MathRow::Element const & e : mrow)
		os << e << "  ";
	return os;
}

} // namespace lyx
