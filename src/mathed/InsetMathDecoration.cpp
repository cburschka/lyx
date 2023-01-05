/**
 * \file InsetMathDecoration.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathDecoration.h"

#include "InsetMathChar.h"
#include "MathData.h"
#include "MathParser.h"
#include "MathSupport.h"
#include "MathStream.h"
#include "MetricsInfo.h"

#include "BufferView.h"
#include "LaTeXFeatures.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include <algorithm>
#include <ostream>

using namespace lyx::support;

using namespace std;

namespace lyx {


InsetMathDecoration::InsetMathDecoration(Buffer * buf, latexkeys const * key)
	: InsetMathNest(buf, 1), key_(key)
{
//	lyxerr << " creating deco " << key->name << endl;
}


Inset * InsetMathDecoration::clone() const
{
	return new InsetMathDecoration(*this);
}


bool InsetMathDecoration::upper() const
{
	return key_->name.substr(0, 5) != "under" && key_->name != "utilde";
}


MathClass InsetMathDecoration::mathClass() const
{
	if (key_->name == "overbrace" || key_->name == "underbrace")
		return MC_OP;
	return MC_ORD;
}


Limits InsetMathDecoration::defaultLimits(bool /*display*/) const
{
	if (allowsLimitsChange())
		return LIMITS;
	else
		return NO_LIMITS;
}


bool InsetMathDecoration::protect() const
{
	return
			key_->name == "overbrace" ||
			key_->name == "underbrace" ||
			key_->name == "overleftarrow" ||
			key_->name == "overrightarrow" ||
			key_->name == "overleftrightarrow" ||
			key_->name == "underleftarrow" ||
			key_->name == "underrightarrow" ||
			key_->name == "underleftrightarrow";
}


bool InsetMathDecoration::wide() const
{
	return
			key_->name == "overline" ||
			key_->name == "underline" ||
			key_->name == "overbrace" ||
			key_->name == "underbrace" ||
			key_->name == "overleftarrow" ||
			key_->name == "overrightarrow" ||
			key_->name == "overleftrightarrow" ||
			key_->name == "widehat" ||
			key_->name == "widetilde" ||
			key_->name == "underleftarrow" ||
			key_->name == "underrightarrow" ||
			key_->name == "underleftrightarrow" ||
			key_->name == "undertilde" ||
			key_->name == "utilde";
}


InsetMath::mode_type InsetMathDecoration::currentMode() const
{
	return key_->name == "underbar" ? TEXT_MODE : MATH_MODE;
}


void InsetMathDecoration::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Changer dummy = mi.base.changeEnsureMath(currentMode());

	cell(0).metrics(mi, dim);

	int const l1 = mi.base.bv->zoomedPixels(1);
	int const l2 = mathed_char_width(mi.base.font, 'x') - l1;
	int const l3 = l2;

	dh_  = l2; //mathed_char_height(LM_TC_VAR, mi, 'I', ascent_, descent_);
	dw_  = l3; //mathed_char_width(LM_TC_VAR, mi, 'x');

	if (upper()) {
		dy_ = -dim.asc - dh_ - l1;
		dim.asc += dh_ + l1;
	} else {
		dy_ = dim.des + l1;
		dim.des += dh_ + l2;
	}
}


void InsetMathDecoration::draw(PainterInfo & pi, int x, int y) const
{
	Changer dummy = pi.base.changeEnsureMath(currentMode());

	cell(0).draw(pi, x, y);
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	if (wide()) {
		mathed_draw_deco(pi, x, y + dy_, dim0.wid, dh_, key_->name);
		return;
	}
	// Lacking the necessary font parameters, in order to properly align
	// the decoration we have to resort to heuristics for choosing a
	// suitable value for shift
	char_type c = (cell(0).empty() || !cell(0)[0]->asCharInset())
		? 0 : cell(0)[0]->asCharInset()->getChar();
	double slope = (c == 0) ? 0.0 : mathed_char_slope(pi.base, c);
	int kerning = (c == 0) ? 0 : mathed_char_kerning(pi.base.font, c);
	int shift = (kerning == 0) ? int(dim0.asc * slope) : kerning;
	mathed_draw_deco(pi, x + (dim0.wid - dw_) / 2 + shift,
	                 y + dy_, dw_, dh_, key_->name);
}


void InsetMathDecoration::write(TeXMathStream & os) const
{
	MathEnsurer ensurer(os);
	if (os.fragile() && protect())
		os << "\\protect";
	os << '\\' << key_->name << '{';
	ModeSpecifier specifier(os, currentMode());
	os << cell(0) << '}';
	writeLimits(os);
}


void InsetMathDecoration::normalize(NormalStream & os) const
{
	os << "[deco " << key_->name << ' ' <<  cell(0) << ']';
}


void InsetMathDecoration::infoize(odocstream & os) const
{
	os << bformat(_("Decoration: %1$s"), key_->name);
}


namespace {
	struct Attributes {
		Attributes() : over(false) {}
		Attributes(bool o, string const & entity)
			: over(o), entity(entity) {}
		bool over;
		string entity;
	};

	typedef map<string, Attributes> TranslationMap;

	void buildTranslationMap(TranslationMap & t) {
		// the decorations we need to support are listed in lib/symbols
		t["acute"] = Attributes(true, "&#x00B4;");
		t["bar"]   = Attributes(true, "&#x00AF;");
		t["breve"] = Attributes(true, "&#x02D8;");
		t["check"] = Attributes(true, "&#x02C7;");
		t["ddddot"] = Attributes(true, "&#x20DC;");
		t["dddot"] = Attributes(true, "&#x20DB;");
		t["ddot"] = Attributes(true, "&#x00A8;");
		t["dot"] = Attributes(true, "&#x02D9;");
		t["grave"] = Attributes(true, "&#x0060;");
		t["hat"] = Attributes(true, "&#x02C6;");
		t["mathring"] = Attributes(true, "&#x02DA;");
		t["overbrace"] = Attributes(true, "&#x23DE;");
		t["overleftarrow"] = Attributes(true, "&#x27F5;");
		t["overleftrightarrow"] = Attributes(true, "&#x27F7;");
		t["overline"] = Attributes(true, "&#x00AF;");
		t["overrightarrow"] = Attributes(true, "&#x27F6;");
		t["tilde"] = Attributes(true, "&#x02DC;");
		t["underbar"] = Attributes(false, "&#x0332;");
		t["underbrace"] = Attributes(false, "&#x23DF;");
		t["underleftarrow"] = Attributes(false, "&#x27F5;");
		t["underleftrightarrow"] = Attributes(false, "&#x27F7;");
		// this is the macron, again, but it works
		t["underline"] = Attributes(false, "&#x00AF;");
		t["underrightarrow"] = Attributes(false, "&#x27F6;");
		t["undertilde"] = Attributes(false, "&#x223C;");
		t["utilde"] = Attributes(false, "&#x223C;");
		t["vec"] = Attributes(true, "&#x2192;");
		t["widehat"] = Attributes(true, "&#x005E;");
		t["widetilde"] = Attributes(true, "&#x223C;");
	}

	TranslationMap const & translationMap() {
		static TranslationMap t;
		if (t.empty())
			buildTranslationMap(t);
		return t;
	}
} // namespace

void InsetMathDecoration::mathmlize(MathMLStream & ms) const
{
	TranslationMap const & t = translationMap();
	TranslationMap::const_iterator cur = t.find(to_utf8(key_->name));
	LASSERT(cur != t.end(), return);
	char const * const outag = cur->second.over ? "mover" : "munder";
	std::string decoration = cur->second.entity;
	ms << MTag(outag)
	   << cell(0)
	   << "<" << from_ascii(ms.namespacedTag("mo")) << " stretchy=\"true\">"
	   << from_ascii(decoration)
	   << "</" << from_ascii(ms.namespacedTag("mo")) << ">"
	   << ETag(outag);
}


void InsetMathDecoration::htmlize(HtmlStream & os) const
{
	string const name = to_utf8(key_->name);
	if (name == "bar") {
		os << MTag("span", "class='overbar'") << cell(0) << ETag("span");
		return;
	}

	if (name == "underbar" || name == "underline") {
		os << MTag("span", "class='underbar'") << cell(0) << ETag("span");
		return;
	}

	TranslationMap const & t = translationMap();
	TranslationMap::const_iterator cur = t.find(name);
	LASSERT(cur != t.end(), return);

	bool symontop = cur->second.over;
	string const symclass = symontop ? "symontop" : "symonbot";
	os << MTag("span", "class='symbolpair " + symclass + "'")
	   << '\n';

	if (symontop)
		os << MTag("span", "class='symbol'") << from_ascii(cur->second.entity);
	else
		os << MTag("span", "class='base'") << cell(0);
	os << ETag("span") << '\n';
	if (symontop)
		os << MTag("span", "class='base'") << cell(0);
	else
		os << MTag("span", "class='symbol'") << from_ascii(cur->second.entity);
	os << ETag("span") << '\n' << ETag("span") << '\n';
}


// ideas borrowed from the eLyXer code
void InsetMathDecoration::validate(LaTeXFeatures & features) const
{
	if (features.runparams().math_flavor == OutputParams::MathAsHTML) {
		string const name = to_utf8(key_->name);
		if (name == "bar") {
			features.addCSSSnippet("span.overbar{border-top: thin black solid;}");
		} else if (name == "underbar" || name == "underline") {
			features.addCSSSnippet("span.underbar{border-bottom: thin black solid;}");
		} else {
			features.addCSSSnippet(
				"span.symbolpair{display: inline-block; text-align:center;}\n"
				"span.symontop{vertical-align: top;}\n"
				"span.symonbot{vertical-align: bottom;}\n"
				"span.symbolpair span{display: block;}\n"
				"span.symbol{height: 0.5ex;}");
		}
	} else {
		if (!key_->required.empty())
			features.require(key_->required);
	}
	InsetMathNest::validate(features);
}

} // namespace lyx
