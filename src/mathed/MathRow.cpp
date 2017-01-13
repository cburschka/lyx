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


MathRow::Element::Element(MetricsInfo const & mi, Type t, MathClass mc)
	: type(t), mclass(mc), before(0), after(0), macro_nesting(mi.base.macro_nesting),
	  marker(InsetMath::NO_MARKER), inset(0), compl_unique_to(0),
	  macro(0), color(Color_red)
{}


MathRow::MathRow(MetricsInfo & mi, MathData const * ar)
{
	// First there is a dummy element of type "open"
	push_back(Element(mi, DUMMY, MC_OPEN));

	// Then insert the MathData argument
	bool const has_contents = ar->addToMathRow(*this, mi);

	// A MathRow should not be completely empty
	if (!has_contents) {
		Element e(mi, BOX, MC_ORD);
		// empty arrays are visible when they are editable
		e.color = mi.base.macro_nesting == 0 ? Color_mathline : Color_none;
		push_back(e);
	}

	// Finally there is a dummy element of type "close"
	push_back(Element(mi, DUMMY, MC_CLOSE));

	/* Do spacing only in math mode. This test is a bit clumsy,
	 * but it is used in other places for guessing the current mode.
	 */
	bool const dospacing = isMathFont(mi.base.fontname);

	// update classes
	if (dospacing) {
		for (int i = 1 ; i != static_cast<int>(elements_.size()) - 1 ; ++i) {
			if (elements_[i].mclass != MC_UNKNOWN)
				update_class(elements_[i].mclass, elements_[before(i)].mclass,
							 elements_[after(i)].mclass);
		}
	}

	// set spacing
	// We go to the end to handle spacing at the end of equation
	for (int i = 1 ; i != static_cast<int>(elements_.size()) ; ++i) {
		Element & e = elements_[i];

		if (e.mclass == MC_UNKNOWN)
			continue;

		Element & bef = elements_[before(i)];
		if (dospacing) {
			int spc = class_spacing(bef.mclass, e.mclass, mi.base);
			bef.after += spc / 2;
			// this is better than spc / 2 to avoid rounding problems
			e.before += spc - spc / 2;
		}

		// finally reserve space for markers
		if (bef.marker != Inset::NO_MARKER)
			bef.after = max(bef.after, 1);
		if (e.marker != Inset::NO_MARKER)
			e.before = max(e.before, 1);
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
	while (elements_[i].mclass == MC_UNKNOWN);

	return i;
}


int MathRow::after(int i) const
{
	do
		++i;
	while (elements_[i].mclass == MC_UNKNOWN);

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
		mi.base.macro_nesting = e.macro_nesting;
		Dimension d;
		switch (e.type) {
		case DUMMY:
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
			if (e.color != Color_none) {
				// allow for one pixel before/after the box.
				d.wid += e.before + e.after + 2;
			} else {
				// hide the box, but give it some height
				d.wid = 0;
			}
			break;
		}

		// handle vertical space for markers
		switch(e.marker) {
		case InsetMath::NO_MARKER:
			break;
		case InsetMath::MARKER:
			++d.des;
			break;
		case InsetMath::MARKER2:
			++d.asc;
			++d.des;
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


namespace {

void drawMarkers(PainterInfo const & pi, MathRow::Element const & e, int const x, int const y)
{
	if (e.marker == InsetMath::NO_MARKER)
		return;

	CoordCache const & coords = pi.base.bv->coordCache();
	Dimension const dim = coords.getInsets().dim(e.inset);

	// the marker is before/after the inset. Normally some space has been reserved already.
	int const l = x + e.before - 1;
	int const r = x + dim.width() - e.after;

	// Duplicated from Inset.cpp and adapted. It is believed that the
	// Inset version should die eventually
	ColorCode pen_color = e.inset->mouseHovered(pi.base.bv) || e.inset->editing(pi.base.bv)?
		Color_mathframe : Color_mathcorners;

	int const d = y + dim.descent();
	pi.pain.line(l, d - 3, l, d, pen_color);
	pi.pain.line(r, d - 3, r, d, pen_color);
	pi.pain.line(l, d, l + 3, d, pen_color);
	pi.pain.line(r - 3, d, r, d, pen_color);

	if (e.marker == InsetMath::MARKER)
		return;

	int const a = y - dim.ascent();
	pi.pain.line(l, a + 3, l, a, pen_color);
	pi.pain.line(r, a + 3, r, a, pen_color);
	pi.pain.line(l, a, l + 3, a, pen_color);
	pi.pain.line(r - 3, a, r, a, pen_color);
}

}

void MathRow::draw(PainterInfo & pi, int x, int const y) const
{
	CoordCache & coords = pi.base.bv->coordCache();
	for (Element const & e : elements_) {
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
			drawMarkers(pi, e, x, y);
			x += d.wid;
			break;
		}
		case BEG_MACRO:
			coords.insets().add(e.macro, x, y);

			drawMarkers(pi, e, x, y);
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
			if (e.color == Color_none)
				break;
			Dimension const d = theFontMetrics(pi.base.font).dimension('I');
			pi.pain.rectangle(x + e.before + 1, y - d.ascent(),
			                  d.width() - 1, d.height() - 1, e.color);
			x += d.wid + 2 + e.before + e.after;
			break;
		}
		case DUMMY:
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
	case MathRow::DUMMY:
		os << (e.mclass == MC_OPEN ? "{" : "}");
		break;
	case MathRow::INSET:
		os << "<" << e.before << "-"
		   << to_utf8(class_to_string(e.mclass))
		   << "-" << e.after << ">";
		break;
	case MathRow::BEG_MACRO:
		os << "\\" << to_utf8(e.macro->name())
		   << "^" << e.macro->nesting() << "[";
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
		os << "<" << e.before << "-[]-" << e.after << ">";
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
