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


bool InsetMathDecoration::isScriptable() const
{
	return
			key_->name == "overbrace" ||
			key_->name == "underbrace";
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
	bool really_change_font = currentMode() == TEXT_MODE
				&& isMathFont(from_ascii(mi.base.fontname));
	FontSetChanger dummy(mi.base, "textnormal", really_change_font);

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

	metricsMarkers(dim);
}


void InsetMathDecoration::draw(PainterInfo & pi, int x, int y) const
{
	bool really_change_font = currentMode() == TEXT_MODE
				&& isMathFont(from_ascii(pi.base.fontname));
	FontSetChanger dummy(pi.base, "textnormal", really_change_font);

	cell(0).draw(pi, x + 1, y);
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	if (wide())
		mathed_draw_deco(pi, x + 1, y + dy_, dim0.wid, dh_, key_->name);
	else
		mathed_draw_deco(pi, x + 1 + (dim0.wid - dw_) / 2,
			y + dy_, dw_, dh_, key_->name);
	drawMarkers(pi, x, y);
	setPosCache(pi, x, y);
}


void InsetMathDecoration::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	if (os.fragile() && protect())
		os << "\\protect";
	os << '\\' << key_->name << '{';
	ModeSpecifier specifier(os, currentMode());
	os << cell(0) << '}';
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
		Attributes() {}
		Attributes(bool o, string t)
			: over(o), tag(t) {}
		bool over;
		string tag;
	};

	typedef map<string, Attributes> TranslationMap;

	void buildTranslationMap(TranslationMap & t) {
		// the decorations we need to support are listed in lib/symbols
		t["acute"] = Attributes(true, "&acute;");
		t["bar"]   = Attributes(true, "&OverBar;");
		t["breve"] = Attributes(true, "&breve;");
		t["check"] = Attributes(true, "&caron;");
		t["ddddot"] = Attributes(true, "&DotDot;");
		t["dddot"] = Attributes(true, "&TripleDot;");
		t["ddot"] = Attributes(true, "&Dot;");
		t["dot"] = Attributes(true, "&dot;");
		t["grave"] = Attributes(true, "&grave;");
		t["hat"] = Attributes(true, "&circ;");
		t["mathring"] = Attributes(true, "&ring;");
		t["overbrace"] = Attributes(true, "&OverBrace;");
		t["overleftarrow"] = Attributes(true, "&xlarr;");
		t["overleftrightarrow"] = Attributes(true, "&xharr;");
		t["overline"] = Attributes(true, "&macr;");
		t["overrightarrow"] = Attributes(true, "&xrarr;");
		t["tilde"] = Attributes(true, "&tilde;");
		t["underbar"] = Attributes(false, "&UnderBar;");
		t["underbrace"] = Attributes(false, "&UnderBrace;");
		t["underleftarrow"] = Attributes(false, "&xlarr;");
		t["underleftrightarrow"] = Attributes(false, "&xharr;");
		// this is the macron, again, but it works
		t["underline"] = Attributes(false, "&macr;");
		t["underrightarrow"] = Attributes(false, "&xrarr;");
		t["undertilde"] = Attributes(false, "&Tilde;");
		t["utilde"] = Attributes(false, "&Tilde;");
		t["vec"] = Attributes(true, "&rarr;");
		t["widehat"] = Attributes(true, "&Hat;");
		t["widetilde"] = Attributes(true, "&Tilde;");
	}

	TranslationMap const & translationMap() {
		static TranslationMap t;
		if (t.empty())
			buildTranslationMap(t);
		return t;
	}
}

void InsetMathDecoration::mathmlize(MathStream & os) const
{
	TranslationMap const & t = translationMap();
	TranslationMap::const_iterator cur = t.find(to_utf8(key_->name));
	LASSERT(cur != t.end(), return);
	char const * const outag = cur->second.over ? "mover" : "munder";
	os << MTag(outag)
		 << MTag("mrow") << cell(0) << ETag("mrow")
		 << from_ascii("<mo stretchy=\"true\">" + cur->second.tag + "</mo>")
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
		if (!key_->requires.empty())
			features.require(key_->requires);
	}
	InsetMathNest::validate(features);
}

} // namespace lyx
