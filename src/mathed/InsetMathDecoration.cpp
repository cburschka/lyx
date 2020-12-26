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

#include "MathData.h"
#include "MathParser.h"
#include "MathSupport.h"
#include "MathStream.h"
#include "MetricsInfo.h"

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


Limits InsetMathDecoration::defaultLimits(bool display) const
{
	if (allowsLimitsChange() && display)
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

	dh_  = 6; //mathed_char_height(LM_TC_VAR, mi, 'I', ascent_, descent_);
	dw_  = 6; //mathed_char_width(LM_TC_VAR, mi, 'x');

	if (upper()) {
		dy_ = -dim.asc - dh_;
		dim.asc += dh_ + 1;
	} else {
		dy_ = dim.des + 1;
		dim.des += dh_ + 2;
	}
}


void InsetMathDecoration::draw(PainterInfo & pi, int x, int y) const
{
	Changer dummy = pi.base.changeEnsureMath(currentMode());

	cell(0).draw(pi, x, y);
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	if (wide())
		mathed_draw_deco(pi, x + 1, y + dy_, dim0.wid, dh_, key_->name);
	else
		mathed_draw_deco(pi, x + 1 + (dim0.wid - dw_) / 2,
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
		Attributes(bool o, string const & t, string const & entity)
			: over(o), tag(t), entity(entity) {}
		bool over;
		string tag;
		string entity;
	};

	typedef map<string, Attributes> TranslationMap;

	void buildTranslationMap(TranslationMap & t) {
		// the decorations we need to support are listed in lib/symbols
		t["acute"] = Attributes(true, "&acute;", "&#x00B4;");
		t["bar"]   = Attributes(true, "&OverBar;", "&#x00AF;");
		t["breve"] = Attributes(true, "&breve;", "&#x02D8;");
		t["check"] = Attributes(true, "&caron;", "&#x02C7;");
		t["ddddot"] = Attributes(true, "&DotDot;", "&#x20DC;");
		t["dddot"] = Attributes(true, "&TripleDot;", "&#x20DB;");
		t["ddot"] = Attributes(true, "&Dot;", "&#x00A8;");
		t["dot"] = Attributes(true, "&dot;", "&#x02D9;");
		t["grave"] = Attributes(true, "&grave;", "&#x0060;");
		t["hat"] = Attributes(true, "&circ;", "&#x02C6;");
		t["mathring"] = Attributes(true, "&ring;", "&#x02DA;");
		t["overbrace"] = Attributes(true, "&OverBrace;", "&#xFE37;");
		t["overleftarrow"] = Attributes(true, "&xlarr;", "&#x27F5;");
		t["overleftrightarrow"] = Attributes(true, "&xharr;", "&#x27F7;");
		t["overline"] = Attributes(true, "&macr;", "&#x00AF;");
		t["overrightarrow"] = Attributes(true, "&xrarr;", "&#x27F6;");
		t["tilde"] = Attributes(true, "&tilde;", "&#x02DC;");
		t["underbar"] = Attributes(false, "&UnderBar;", "&#x0332;");
		t["underbrace"] = Attributes(false, "&UnderBrace;", "&#xFE38;");
		t["underleftarrow"] = Attributes(false, "&xlarr;", "&#x27F5;");
		t["underleftrightarrow"] = Attributes(false, "&xharr;", "&#x27F7;");
		// this is the macron, again, but it works
		t["underline"] = Attributes(false, "&macr;", "&#x00AF;");
		t["underrightarrow"] = Attributes(false, "&xrarr;", "&#x27F6;");
		t["undertilde"] = Attributes(false, "&Tilde;", "&#x223C;");
		t["utilde"] = Attributes(false, "&Tilde;", "&#x223C;");
		t["vec"] = Attributes(true, "&rarr;", "&#x2192;");
		t["widehat"] = Attributes(true, "&Hat;", "&#x005E;");
		t["widetilde"] = Attributes(true, "&Tilde;", "&#x223C;");
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
	std::string decoration = ms.xmlMode() ? cur->second.entity : cur->second.tag;
	ms << MTag(outag)
	   << MTag("mrow") << cell(0) << ETag("mrow")
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
		os << MTag("span", "class='symbol'") << from_ascii(cur->second.tag);
	else
		os << MTag("span", "class='base'") << cell(0);
	os << ETag("span") << '\n';
	if (symontop)
		os << MTag("span", "class='base'") << cell(0);
	else
		os << MTag("span", "class='symbol'") << from_ascii(cur->second.tag);
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
