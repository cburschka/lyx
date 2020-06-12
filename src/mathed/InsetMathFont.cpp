/**
 * \file InsetMathFont.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathFont.h"

#include "LaTeXFeatures.h"
#include "MathData.h"
#include "MathStream.h"
#include "MathParser.h"
#include "MetricsInfo.h"

#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include <ostream>

using namespace lyx::support;

namespace lyx {

InsetMathFont::InsetMathFont(Buffer * buf, latexkeys const * key)
	: InsetMathNest(buf, 1), key_(key)
{}


Inset * InsetMathFont::clone() const
{
	return new InsetMathFont(*this);
}


std::string InsetMathFont::font() const
{
	LASSERT(isAscii(key_->name), return "mathnormal");
	return to_ascii(key_->name);
}


InsetMath::mode_type InsetMathFont::currentMode() const
{
	if (key_->extra == "mathmode")
		return MATH_MODE;
	if (key_->extra == "textmode")
		return TEXT_MODE;
	return UNDECIDED_MODE;
}


bool InsetMathFont::lockedMode() const
{
	if (key_->extra == "forcetext")
		return true;
	return false;
}


void InsetMathFont::write(WriteStream & os) const
{
	// Close the mode changing command inserted during export if
	// we are going to output another mode changing command that
	// actually doesn't change mode. This avoids exporting things
	// such as \ensuremath{a\mathit{b}} or \textit{a\text{b}} and
	// produce instead \ensuremath{a}\mathit{b} and \textit{a}\text{b}.
	if (os.pendingBrace()
	    && ((currentMode() == TEXT_MODE && os.textMode())
		    || (currentMode() == MATH_MODE && !os.textMode()))) {
		os.os() << '}';
		os.pendingBrace(false);
		os.textMode(!os.textMode());
	}
	InsetMathNest::write(os);
}


void InsetMathFont::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Changer dummy = mi.base.changeFontSet(font());
	cell(0).metrics(mi, dim);
}


void InsetMathFont::draw(PainterInfo & pi, int x, int y) const
{
	Changer dummy = pi.base.changeFontSet(font());
	cell(0).draw(pi, x, y);
}


void InsetMathFont::metricsT(TextMetricsInfo const & mi, Dimension &) const
{
	// FIXME: BROKEN!
	Dimension dim;
	cell(0).metricsT(mi, dim);
}


void InsetMathFont::drawT(TextPainter & pain, int x, int y) const
{
	cell(0).drawT(pain, x, y);
}


docstring InsetMathFont::name() const
{
	return key_->name;
}


void InsetMathFont::validate(LaTeXFeatures & features) const
{
	InsetMathNest::validate(features);
	std::string fontname = font();
	if (features.runparams().isLaTeX()) {
		// Make sure amssymb is put in preamble if Blackboard Bold or
		// Fraktur used:
		if (fontname == "mathfrak" || fontname == "mathbb")
			features.require("amssymb");
		if (fontname == "text" || fontname == "textnormal"
		    || (fontname.length() == 6 && fontname.substr(0, 4) == "text"))
			features.require("amstext");
		if (fontname == "mathscr")
			features.require("mathrsfs");
		if (fontname == "textipa")
			features.require("tipa");
		if (fontname == "ce" || fontname == "cf")
			features.require("mhchem");
		if (fontname == "mathds")
			features.require("dsfont");
	} else if (features.runparams().math_flavor == OutputParams::MathAsHTML) {
		features.addCSSSnippet(
			"span.normal{font: normal normal normal inherit serif;}\n"
			"span.fraktur{font: normal normal normal inherit cursive;}\n"
			"span.bold{font: normal normal bold inherit serif;}\n"
			"span.script{font: normal normal normal inherit cursive;}\n"
			"span.italic{font: italic normal normal inherit serif;}\n"
			"span.sans{font: normal normal normal inherit sans-serif;}\n"
			"span.monospace{font: normal normal normal inherit monospace;}\n"
			"span.noun{font: normal small-caps normal inherit normal;}");
	}
}


// The fonts we want to support are listed in lib/symbols
void InsetMathFont::htmlize(HtmlStream & os) const
{
	// FIXME These are not quite right, because they do not nest
	// correctly. A proper fix would presumably involve tracking
	// the fonts already in effect.
	std::string variant;
	docstring const & tag = key_->name;
	if (tag == "mathnormal" || tag == "mathrm"
	    || tag == "text" || tag == "textnormal"
	    || tag == "textrm" || tag == "textup"
	    || tag == "textmd")
		variant = "normal";
	else if (tag == "frak" || tag == "mathfrak")
		variant = "fraktur";
	else if (tag == "mathbf" || tag == "textbf")
		variant = "bold";
	else if (tag == "mathbb" || tag == "mathbbm"
	         || tag == "mathds")
		variant = "double-struck";
	else if (tag == "mathcal")
		variant = "script";
	else if (tag == "mathit" || tag == "textsl"
	         || tag == "emph" || tag == "textit")
		variant = "italic";
	else if (tag == "mathsf" || tag == "textsf")
		variant = "sans";
	else if (tag == "mathtt" || tag == "texttt")
		variant = "monospace";
	else if (tag == "textipa" || tag == "textsc" || tag == "noun")
		variant = "noun";

	docstring const beg = (tag.size() < 4) ? from_ascii("") : tag.substr(0, 4);
	if (!variant.empty()) {
		os << MTag("span", "class='" + variant + "'")
		   << cell(0)
		   << ETag("span");
	} else
		os << cell(0);
}


// The fonts we want to support are listed in lib/symbols
void InsetMathFont::mathmlize(MathStream & os) const
{
	// FIXME These are not quite right, because they do not nest
	// correctly. A proper fix would presumably involve tracking
	// the fonts already in effect.
	std::string variant;
	docstring const & tag = key_->name;
	if (tag == "mathnormal" || tag == "mathrm"
	    || tag == "text" || tag == "textnormal"
	    || tag == "textrm" || tag == "textup"
	    || tag == "textmd")
		variant = "normal";
	else if (tag == "frak" || tag == "mathfrak")
		variant = "fraktur";
	else if (tag == "mathbf" || tag == "textbf")
		variant = "bold";
	else if (tag == "mathbb" || tag == "mathbbm"
	         || tag == "mathds")
		variant = "double-struck";
	else if (tag == "mathcal")
		variant = "script";
	else if (tag == "mathit" || tag == "textsl"
	         || tag == "emph" || tag == "textit")
		variant = "italic";
	else if (tag == "mathsf" || tag == "textsf")
		variant = "sans-serif";
	else if (tag == "mathtt" || tag == "texttt")
		variant = "monospace";
	// no support at present for textipa, textsc, noun

	if (!variant.empty())
		os << MTag("mstyle", "mathvariant='" + variant + "'")
		   << cell(0)
		   << ETag("mstyle");
	else
		os << cell(0);
}


void InsetMathFont::infoize(odocstream & os) const
{
	os << bformat(_("Font: %1$s"), key_->name);
}


} // namespace lyx
