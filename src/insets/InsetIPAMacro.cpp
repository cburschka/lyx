/**
 * \file InsetIPAMacro.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetIPAMacro.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Dimension.h"
#include "Encoding.h"
#include "Font.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "output_xhtml.h"
#include "texstream.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/Translator.h"

using namespace std;

namespace lyx {

namespace {

typedef Translator<string, InsetIPADecoParams::Type> IPADecoTranslator;
typedef Translator<docstring, InsetIPADecoParams::Type> IPADecoTranslatorLoc;

IPADecoTranslator const init_ipadecotranslator()
{
	IPADecoTranslator translator("toptiebar", InsetIPADecoParams::Toptiebar);
	translator.addPair("bottomtiebar", InsetIPADecoParams::Bottomtiebar);
	return translator;
}


IPADecoTranslatorLoc const init_ipadecotranslator_loc()
{
	IPADecoTranslatorLoc translator(_("Top tie bar"), InsetIPADecoParams::Toptiebar);
	translator.addPair(_("Bottom tie bar"), InsetIPADecoParams::Bottomtiebar);
	return translator;
}


IPADecoTranslator const & ipadecotranslator()
{
	static IPADecoTranslator const decotranslator =
			init_ipadecotranslator();
	return decotranslator;
}


IPADecoTranslatorLoc const & ipadecotranslator_loc()
{
	static IPADecoTranslatorLoc const translator =
	    init_ipadecotranslator_loc();
	return translator;
}


typedef Translator<string, InsetIPAChar::Kind> IPACharTranslator;

IPACharTranslator const init_ipachartranslator()
{
	IPACharTranslator translator("\\tone{51}", InsetIPAChar::TONE_FALLING);
	translator.addPair("\\tone{15}", InsetIPAChar::TONE_RISING);
	translator.addPair("\\tone{45}", InsetIPAChar::TONE_HIGH_RISING);
	translator.addPair("\\tone{12}", InsetIPAChar::TONE_LOW_RISING);
	translator.addPair("\\tone{454}", InsetIPAChar::TONE_HIGH_RISING_FALLING);
	return translator;
}


IPACharTranslator const & ipachartranslator()
{
	static IPACharTranslator const chartranslator =
	    init_ipachartranslator();
	return chartranslator;
}

} // anon


InsetIPADecoParams::InsetIPADecoParams()
	: type(Bottomtiebar)
{}


void InsetIPADecoParams::write(ostream & os) const
{
	string const label = ipadecotranslator().find(type);
	os << "IPADeco " << label << "\n";
}


void InsetIPADecoParams::read(Lexer & lex)
{
	string label;
	lex >> label;
	if (lex)
		type = ipadecotranslator().find(label);
}


/////////////////////////////////////////////////////////////////////
//
// InsetIPADeco
//
/////////////////////////////////////////////////////////////////////

InsetIPADeco::InsetIPADeco(Buffer * buf, string const & label)
	: InsetCollapsable(buf)
{
	setDrawFrame(true);
	setFrameColor(Color_insetframe);
	params_.type = ipadecotranslator().find(label);
}


InsetIPADeco::~InsetIPADeco()
{}


docstring InsetIPADeco::layoutName() const
{
	return from_ascii("IPADeco:" + ipadecotranslator().find(params_.type));
}


void InsetIPADeco::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetText::metrics(mi, dim);
	
	if (params_.type == InsetIPADecoParams::Toptiebar) {
		// consider width of the inset label
		FontInfo font(getLayout().labelfont());
		font.realize(sane_font);
		font.decSize();
		font.decSize();
		int w = 0;
		int a = 0;
		int d = 0;
		docstring const label(1, char_type(0x2040));
		theFontMetrics(font).rectText(label, w, a, d);
		dim.asc += int(a * 0.5);
	}
	if (params_.type == InsetIPADecoParams::Bottomtiebar) {
		// consider width of the inset label
		FontInfo font(getLayout().labelfont());
		font.realize(sane_font);
		font.decSize();
		font.decSize();
		int w = 0;
		int a = 0;
		int d = 0;
		docstring const label(1, char_type(0x203f));
		theFontMetrics(font).rectText(label, w, a, d);
		dim.des += int(d * 1.5);
	}

	// cache the inset dimension
	setDimCache(mi, dim);
}


void InsetIPADeco::draw(PainterInfo & pi, int x, int y) const
{
	// draw the text
	InsetCollapsable::draw(pi, x, y);

	// draw the inset marker
	drawMarkers(pi, x, y);

	Dimension const dim = Inset::dimension(*pi.base.bv);

	if (params_.type == InsetIPADecoParams::Toptiebar) {
		FontInfo font(getLayout().labelfont());
		font.realize(sane_font);
		font.decSize();
		font.decSize();
		int w = 0;
		int a = 0;
		int d = 0;
		int asc = dim.ascent();
		docstring const label(1, char_type(0x2040));
		theFontMetrics(font).rectText(label, w, a, d);
		int const ww = max(dim.wid, w);
		pi.pain.rectText(x + (ww - w) / 2, y - int(asc / 2.5),
			label, font, Color_none, Color_none);
	}

	if (params_.type == InsetIPADecoParams::Bottomtiebar) {
		FontInfo font(getLayout().labelfont());
		font.realize(sane_font);
		font.decSize();
		font.decSize();
		int w = 0;
		int a = 0;
		int d = 0;
		int desc = dim.descent();
		docstring const label(1, char_type(0x203f));
		theFontMetrics(font).rectText(label, w, a, d);
		int const ww = max(dim.wid, w);
		pi.pain.rectText(x + (ww - w) / 2, y + int(desc / 1.5),
			label, font, Color_none, Color_none);
	}
}


void InsetIPADeco::write(ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(os);
}


void InsetIPADeco::read(Lexer & lex)
{
	params_.read(lex);
	InsetCollapsable::read(lex);
}


void InsetIPADeco::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {
	case LFUN_QUOTE_INSERT: {
		FuncRequest fr(LFUN_SELF_INSERT, "\"");
		InsetText::doDispatch(cur, fr);
		break;
	}
	default:
		InsetText::doDispatch(cur, cmd);
		break;
	}
}


bool InsetIPADeco::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action()) {
	case LFUN_SCRIPT_INSERT: {
		if (cmd.argument() == "subscript") {
			flag.setEnabled(false);
			return true;
		}
		break;
	}
	case LFUN_IN_IPA:
		flag.setEnabled(true);
		return true;
		break;
	default:
		break;
	}
	return InsetText::getStatus(cur, cmd, flag);
}


void InsetIPADeco::latex(otexstream & os, OutputParams const & runparams) const
{
	if (params_.type == InsetIPADecoParams::Toptiebar)
		os << "\\texttoptiebar{";
	else if (params_.type == InsetIPADecoParams::Bottomtiebar)
		os << "\\textbottomtiebar{";
	InsetCollapsable::latex(os, runparams);
	os << "}";
}


int InsetIPADeco::plaintext(odocstringstream & os,
			    OutputParams const & runparams, size_t max_length) const
{
	odocstringstream ods;
	int h = (int)(InsetCollapsable::plaintext(ods, runparams, max_length) / 2);
	docstring result = ods.str();
	docstring const before = result.substr(0, h);
	docstring const after = result.substr(h, result.size());
	
	if (params_.type == InsetIPADecoParams::Toptiebar) {
		os << before;
		os.put(0x0361);
		os << after;
	}
	else if (params_.type == InsetIPADecoParams::Bottomtiebar) {
		os << before;
		os.put(0x035c);
		os << after;
	}
	return result.size();
}


int InsetIPADeco::docbook(odocstream & os, OutputParams const & runparams) const
{
	// FIXME: Any docbook option here?
	return InsetCollapsable::docbook(os, runparams);
}


docstring InsetIPADeco::xhtml(XHTMLStream & xs, OutputParams const & runparams) const
{
	// FIXME: Like in plaintext, the combining characters "&#x361;" (toptiebar)
	// or "&#x35c;" (bottomtiebar) would need to be inserted just in the mid
	// of the text string. (How) can this be done with the xhtml stream?
	return InsetCollapsable::xhtml(xs, runparams);
}


docstring InsetIPADeco::toolTip(BufferView const &, int, int) const
{
	return ipadecotranslator_loc().find(params_.type);
}


string InsetIPADeco::params2string(InsetIPADecoParams const & params)
{
	ostringstream data;
	data << "IPADeco" << ' ';
	params.write(data);
	return data.str();
}


void InsetIPADeco::string2params(string const & in, InsetIPADecoParams & params)
{
	params = InsetIPADecoParams();

	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex;
	lex.setStream(data);
	lex.setContext("InsetIPADeco::string2params");
	lex >> "IPADeco" >> "toptiebar";

	params.read(lex);
}


void InsetIPADeco::validate(LaTeXFeatures & features) const
{
	features.require("tipa");
	InsetText::validate(features);
}


bool InsetIPADeco::insetAllowed(InsetCode code) const
{
	switch (code) {
	// code that is allowed
	case ERT_CODE:
	case IPACHAR_CODE:
	case SCRIPT_CODE:
		return true;
	default:
		return false;
	}
}

/////////////////////////////////////////////////////////////////////////
//
// InsetIPAChar
//
/////////////////////////////////////////////////////////////////////////


InsetIPAChar::InsetIPAChar(Kind k)
	: Inset(0), kind_(k)
{}


InsetIPAChar::Kind InsetIPAChar::kind() const
{
	return kind_;
}


void InsetIPAChar::metrics(MetricsInfo & mi, Dimension & dim) const
{
	frontend::FontMetrics const & fm =
		theFontMetrics(mi.base.font);
	dim.asc = fm.maxAscent();
	dim.des = fm.maxDescent();

	string s;
	switch (kind_) {
		case TONE_FALLING:
		case TONE_RISING:
		case TONE_HIGH_RISING:
		case TONE_LOW_RISING:
		case TONE_HIGH_RISING_FALLING:
			s = "_";
			break;
	}
	docstring ds(s.begin(), s.end());
	dim.wid = fm.width(ds);
	setDimCache(mi, dim);
}


void InsetIPAChar::draw(PainterInfo & pi, int x, int y) const
{
	FontInfo font = pi.base.font;
	frontend::FontMetrics const & fm =
			theFontMetrics(font);

	switch (kind_) {
	case TONE_FALLING:
	{
		int w = fm.width(char_type('-'));
		int h = fm.ascent(char_type('M'));
		int x2 = x + w;
		int y2 = y - h;

		pi.pain.line(x2, y2, x2, y, Color_foreground);
		pi.pain.line(x2, y, x, y2, Color_foreground);
		break;
	}
	case TONE_RISING:
	{
		int w = fm.width(char_type('-'));
		int h = fm.ascent(char_type('M'));
		int x2 = x + w;
		int y2 = y - h;

		pi.pain.line(x2, y, x2, y2, Color_foreground);
		pi.pain.line(x2, y2, x, y, Color_foreground);
		break;
	}
	case TONE_HIGH_RISING:
	{
		int w = fm.width(char_type('-'));
		int h = fm.ascent(char_type('M'));
		int x2 = x + w;
		int y2 = y - h;
		int y3 = y - int(h * 0.75);

		pi.pain.line(x2, y, x2, y2, Color_foreground);
		pi.pain.line(x2, y2, x, y3, Color_foreground);
		break;
	}
	case TONE_LOW_RISING:
	{
		int w = fm.width(char_type('-'));
		int h = fm.ascent(char_type('M'));
		int x2 = x + w;
		int y2 = y - h;
		int y3 = y - int(h * 0.25);

		pi.pain.line(x2, y, x2, y2, Color_foreground);
		pi.pain.line(x2, y3, x, y, Color_foreground);
		break;
	}
	case TONE_HIGH_RISING_FALLING:
	{
		int w = fm.width(char_type('-'));
		int h = fm.ascent(char_type('M'));
		int x2 = x + w;
		int y2 = y - h;
		int x3 = x + int(w * 0.5);
		int y3 = y - int(h * 0.75);

		pi.pain.line(x2, y, x2, y2, Color_foreground);
		pi.pain.line(x2, y3, x3, y2, Color_foreground);
		pi.pain.line(x3, y2, x, y3, Color_foreground);
		break;
	}
	}
}


void InsetIPAChar::write(ostream & os) const
{
	string const command = ipachartranslator().find(kind_);
	if (command.empty()) {
		LYXERR0("InsetIPAChar::write: Unknown type");
		return;
	}
	os << "\\IPAChar " << command << "\n";
}


void InsetIPAChar::read(Lexer & lex)
{
	lex.next();
	string const command = lex.getString();
	kind_ = ipachartranslator().find(command);
}


void InsetIPAChar::latex(otexstream & os,
			 OutputParams const &) const
{
	string const command = ipachartranslator().find(kind_);
	os << command;
}


int InsetIPAChar::plaintext(odocstringstream & os, OutputParams const &, size_t) const
{
	switch (kind_) {
	case TONE_FALLING:
		os.put(0x02e5);
		os.put(0x02e9);
		return 2;
	case TONE_RISING:
		os.put(0x02e9);
		os.put(0x02e5);
		return 2;
	case TONE_HIGH_RISING:
		os.put(0x02e7);
		os.put(0x02e5);
		return 2;
	case TONE_LOW_RISING:
		os.put(0x02e9);
		os.put(0x02e7);
		return 2;
	case TONE_HIGH_RISING_FALLING:
		os.put(0x02e8);
		os.put(0x02e5);
		os.put(0x02e8);
		return 3;
	}
	return 0;
}


int InsetIPAChar::docbook(odocstream & /*os*/, OutputParams const &) const
{
	switch (kind_) {
	// FIXME
	LYXERR0("IPA tone macros not yet implemented with DocBook!");
	case TONE_FALLING:
	case TONE_RISING:
	case TONE_HIGH_RISING:
	case TONE_LOW_RISING:
	case TONE_HIGH_RISING_FALLING:
		break;
	}
	return 0;
}


docstring InsetIPAChar::xhtml(XHTMLStream & xs, OutputParams const &) const
{
	switch (kind_) {
	case TONE_FALLING:
		xs << XHTMLStream::ESCAPE_NONE << "&#x2e5;"
		   << XHTMLStream::ESCAPE_NONE << "&#x2e9;";
		break;
	case TONE_RISING:
		xs << XHTMLStream::ESCAPE_NONE << "&#x2e9;"
		   << XHTMLStream::ESCAPE_NONE << "&#x2e5;";
		break;
	case TONE_HIGH_RISING:
		xs << XHTMLStream::ESCAPE_NONE << "&#x2e7;"
		   << XHTMLStream::ESCAPE_NONE << "&#x2e5;";
		break;
	case TONE_LOW_RISING:
		xs << XHTMLStream::ESCAPE_NONE << "&#x2e9;"
		   << XHTMLStream::ESCAPE_NONE << "&#x2e7;";
		break;
	case TONE_HIGH_RISING_FALLING:
		xs << XHTMLStream::ESCAPE_NONE << "&#x2e8;"
		   << XHTMLStream::ESCAPE_NONE << "&#x2e5;"
		   << XHTMLStream::ESCAPE_NONE << "&#x2e8;";
		break;
	}
	return docstring();
}


void InsetIPAChar::toString(odocstream & os) const
{
	odocstringstream ods;
	plaintext(ods, OutputParams(0));
	os << ods.str();
}


void InsetIPAChar::forOutliner(docstring & os, size_t const, bool const) const
{
	odocstringstream ods;
	plaintext(ods, OutputParams(0));
	os += ods.str();
}


void InsetIPAChar::validate(LaTeXFeatures & features) const
{
	switch (kind_) {
	case TONE_FALLING:
	case TONE_RISING:
	case TONE_HIGH_RISING:
	case TONE_LOW_RISING:
	case TONE_HIGH_RISING_FALLING:
		features.require("tone");
		break;
	default:
		break;
	}
}


} // namespace lyx
