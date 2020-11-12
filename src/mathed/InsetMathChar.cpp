/**
 * \file InsetMathChar.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathChar.h"

#include "MathParser.h"
#include "MathSupport.h"
#include "MathStream.h"
#include "MetricsInfo.h"

#include "Dimension.h"
#include "BufferEncodings.h"
#include "LaTeXFeatures.h"
#include "TextPainter.h"

#include "frontends/FontMetrics.h"

#include "support/debug.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include <algorithm>

using namespace std;


namespace lyx {

extern bool has_math_fonts;


namespace {
latexkeys const * makeSubstitute(char_type c)
{
	std::string name;
	switch (c) {
	// Latex replaces ', *, -, and : with specific symbols. With unicode-math,
	// these symbols are replaced respectively by ^U+2032, U+2217, U+2212 and
	// U+2236 (the latter substitution can be turned off with a package
	// option). Unicode-math also replaces ` with \backprime.
		// prime needs to be placed in superscript unless an opentype font is used.
		//case '\'':
		//name = "prime";
		//break;
	case '*':
		name = "ast";
		break;
	case '-':
		name = "lyxminus";// unicode-math: "minus"
		break;
	case ':':
		name = "ordinarycolon";// unicode-math: "mathratio"
		break;
	// The remaining replacements are not real character substitutions (from a
	// unicode point of view) but are done here: 1. for cosmetic reasons, in the
	// context of being stuck with CM fonts at the moment, to ensure consistency
	// with related symbols: -, \leq, \geq, etc.  2. to get the proper spacing
	// as defined in lib/symbols.
	case '+':
		name = "lyxplus";//unicode-math: "mathplus"
		break;
	case '>':
		name = "lyxgt";//unicode-math: "greater"
		break;
	case '<':
		name = "lyxlt";//unicode-math: "less"
		break;
	case '=':
		name = "lyxeqrel";//unicode-math: "equal"
		break;
	//case ','://unicode-math: "mathcomma"
	//case ';'://unicode-math: "mathsemicolon"
	default:
		return nullptr;
	}
	return in_word_set(from_ascii(name));
}

} //anonymous namespace


static bool slanted(char_type c)
{
	return isAlphaASCII(c) || Encodings::isMathAlpha(c);
}


InsetMathChar::InsetMathChar(char_type c)
	: char_(c), kerning_(0), subst_(makeSubstitute(c))
{}



Inset * InsetMathChar::clone() const
{
	return new InsetMathChar(*this);
}


void InsetMathChar::metrics(MetricsInfo & mi, Dimension & dim) const
{
	string const & f = mi.base.fontname;
	if (isMathFont(f) && subst_) {
		// If the char has a substitute, draw the replacement symbol
		// instead, but only in math mode.
		kerning_ = mathedSymbolDim(mi.base, dim, subst_);
	} else if (!slanted(char_) && f == "mathnormal") {
		Changer dummy = mi.base.font.changeShape(UP_SHAPE);
		dim = theFontMetrics(mi.base.font).dimension(char_);
		kerning_ = 0;
	} else if (!isASCII(char_) && Encodings::unicodeCharInfo(char_).isUnicodeSymbol()) {
		Changer dummy1 = mi.base.changeFontSet("mathnormal");
		Changer dummy2 = Encodings::isMathAlpha(char_)
				? noChange()
				: mi.base.font.changeShape(UP_SHAPE);
		dim = theFontMetrics(mi.base.font).dimension(char_);
		kerning_ = -mathed_char_kerning(mi.base.font, char_);
	} else {
		frontend::FontMetrics const & fm = theFontMetrics(mi.base.font);
		dim = fm.dimension(char_);
		kerning_ = mathed_char_kerning(mi.base.font, char_);
		// cmmi has almost no left bearing: simulate this
		if (f == "mathnormal") {
			dim.wid += max(-fm.lbearing(char_), 0);
		}
		// Rule 17 from Appendix G
		// These are the fonts with fontdimen(2)==0.
		// To properly handle the case fontdimen(2)!=0 (that is for all other
		// math fonts), where the italic correction must also be converted into
		// kerning but only at the end of strings of characters with the same
		// font, one would need a more elaborate implementation in MathRow. For
		// now the case fontdimen(2)==0 is the most important.
		if (f == "mathnormal" || f == "mathscr" || f == "mathcal") {
			dim.wid += kerning_;
			// We use a negative value to tell InsetMathScript to move the
			// subscript leftwards instead of the superscript rightwards
			kerning_ = -kerning_;
		}
	}
}


void InsetMathChar::draw(PainterInfo & pi, int x, int y) const
{
	//lyxerr << "drawing '" << char_ << "' font: " << pi.base.fontname << std::endl;
	if (isMathFont(pi.base.fontname)) {
		if (subst_) {
			// If the char has a substitute, draw the replacement symbol
			// instead, but only in math mode.
			mathedSymbolDraw(pi, x, y, subst_);
			return;
		} else if (!slanted(char_) && pi.base.fontname == "mathnormal") {
			Changer dummy = pi.base.font.changeShape(UP_SHAPE);
			pi.draw(x, y, char_);
			return;
		} else if (!isASCII(char_) && Encodings::unicodeCharInfo(char_).isUnicodeSymbol()) {
			Changer dummy1 = pi.base.changeFontSet("mathnormal");
			Changer dummy2 = Encodings::isMathAlpha(char_)
					? noChange()
					: pi.base.font.changeShape(UP_SHAPE);
			pi.draw(x, y, char_);
			return;
		}
	}
	// cmmi has almost no left bearing: simulate this
	if (pi.base.fontname == "mathnormal") {
		x += max(-theFontMetrics(pi.base.font).lbearing(char_), 0);
	}
	pi.draw(x, y, char_);
}


void InsetMathChar::metricsT(TextMetricsInfo const &, Dimension & dim) const
{
	dim.wid = 1;
	dim.asc = 1;
	dim.des = 0;
}


void InsetMathChar::drawT(TextPainter & pain, int x, int y) const
{
	//lyxerr << "drawing text '" << char_ << "' code: " << code_ << endl;
	pain.draw(x, y, char_);
}


void InsetMathChar::write(WriteStream & os) const
{
	os.os().put(char_);
}


void InsetMathChar::validate(LaTeXFeatures & features) const
{
	if (!isASCII(char_))
		BufferEncodings::validate(char_, features, true);
}


void InsetMathChar::normalize(NormalStream & os) const
{
	os << "[char ";
	os.os().put(char_);
	os << " mathalpha]";
}


void InsetMathChar::octave(OctaveStream & os) const
{
	os.os().put(char_);
}


// We have a bit of a problem here. MathML wants to know whether the
// character represents an "identifier" or an "operator", and we have
// no general way of telling. So we shall guess: If it's alpha or
// mathalpha, then we'll treat it as an identifier, otherwise as an
// operator.
// Worst case: We get bad spacing, or bad italics.
void InsetMathChar::mathmlize(MathStream & ms) const
{
	std::string entity;
	switch (char_) {
		case '<': entity = "&lt;"; break;
		case '>': entity = "&gt;"; break;
		case '&': entity = "&amp;"; break;
		case ' ': {
			if (ms.xmlMode())
				ms << from_ascii("&#0160;");
			else
				ms << from_ascii("&nbsp;");
			return;
		}
		default: break;
	}

	if (ms.inText()) {
		if (entity.empty())
			ms.os().put(char_);
		else
			ms << from_ascii(entity);
		return;
	}

	if (!entity.empty()) {
		ms << "<" << from_ascii(ms.namespacedTag("mo")) << ">"
		   << from_ascii(entity)
		   << "</" << from_ascii(ms.namespacedTag("mo")) << ">";
		return;
	}

	char const * type =
		(isAlphaASCII(char_) || Encodings::isMathAlpha(char_))
			? "mi" : "mo";
	// we don't use MTag and ETag because we do not want the spacing before the end tag.
	docstring tag = from_ascii(ms.namespacedTag(type));
	ms << "<" << tag << ">" << char_type(char_) << "</" << tag << ">";
}


void InsetMathChar::htmlize(HtmlStream & ms) const
{
	std::string entity;
	// Not taking subst_ into account here because the MathML output of
	// <>=+-* looks correct as it is. FIXME: ' is not output as ^\prime
	switch (char_) {
		case '<': entity = "&lt;"; break;
		case '>': entity = "&gt;"; break;
		case '&': entity = "&amp;"; break;
		case ' ': entity = "&nbsp;"; break;
		default: break;
	}

	bool have_entity = !entity.empty();

	if (ms.inText()) {
		if (have_entity)
			ms << from_ascii(entity);
		else
			ms.os().put(char_);
		return;
	}

	if (have_entity) {
		// an operator, so give some space
		ms << ' ' << from_ascii(entity) << ' ';
		return;
	}

	if (isAlphaASCII(char_) || Encodings::isMathAlpha(char_))
		// we don't use MTag and ETag because we do not want the spacing
		ms << MTag("i") << char_type(char_) << ETag("i");
	else
		// an operator, so give some space
		ms << " " << char_type(char_) << " ";
}


MathClass InsetMathChar::mathClass() const
{
	// this information comes from fontmath.ltx in LaTeX source.
	char const ch = static_cast<char>(char_);
	if (subst_)
		return string_to_class(subst_->extra);
	else if (support::contains(",;", ch))
		return MC_PUNCT;
	else if (support::contains("([", ch))
		return MC_OPEN;
	else if (support::contains(")]!?", ch))
		return MC_CLOSE;
	else return MC_ORD;
}


} // namespace lyx
