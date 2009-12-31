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
#include "support/lassert.h"

#include <ostream>

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
	return key_->name.substr(0, 5) != "under";
}


bool InsetMathDecoration::isScriptable() const
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
			key_->name == "underleftrightarrow";
}


void InsetMathDecoration::metrics(MetricsInfo & mi, Dimension & dim) const
{
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
	os << '\\' << key_->name << '{' << cell(0) << '}';
}


void InsetMathDecoration::normalize(NormalStream & os) const
{
	os << "[deco " << key_->name << ' ' <<  cell(0) << ']';
}


void InsetMathDecoration::infoize(odocstream & os) const
{
	os << "Deco: " << key_->name;
}


void InsetMathDecoration::validate(LaTeXFeatures & features) const
{
	if (!key_->requires.empty())
		features.require(to_utf8(key_->requires));
	InsetMathNest::validate(features);
}

namespace {
	struct Attributes {
		Attributes() {}
		Attributes(bool o, string t)
			: over(o), tag(t) {}
		bool over;
		string tag;
	};

	typedef map<string, Attributes> Translator;

	void buildTranslator(Translator & t) {
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
		t["overrightarrow"] = Attributes(true, "&xrarr;");
		t["tilde"] = Attributes(true, "&tilde;");
		t["underbar"] = Attributes(false, "&UnderBar;");
		t["underbrace"] = Attributes(false, "&UnderBrace;");
		t["underleftarrow"] = Attributes(false, "&xlarr;");
		t["underleftrightarrow"] = Attributes(false, "&xharr;");
		t["underline"] = Attributes(false, "&;");
		t["underrightarrow"] = Attributes(false, "&xrarr;");
		t["vec"] = Attributes(true, "&rarr;");
		t["widehat"] = Attributes(true, "&Hat;");
		t["widetilde"] = Attributes(true, "&Tilde;");
	}

	Translator const & translator() {
		static Translator t;
		if (t.empty())
			buildTranslator(t);
		return t;
	}
}

void InsetMathDecoration::mathmlize(MathStream & os) const
{
	Translator const & t = translator();
	Translator::const_iterator cur = t.find(to_utf8(key_->name));
	LASSERT(cur != t.end(), return);
	char const * const outag = cur->second.over ? "mover" : "munder";
	os << MTag(outag)
		 << MTag("mrow") << cell(0) << ETag("mrow")
		 << from_ascii("<mo stretchy=\"true\">" + cur->second.tag + "</mo>")
		 << ETag(outag);
}


} // namespace lyx
