/**
 * \file InsetScript.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetScript.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "Dimension.h"
#include "DispatchResult.h"
#include "Exporter.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "LyXAction.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "output_xhtml.h"
#include "TextClass.h"
#include "TextMetrics.h"

#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Translator.h"

#include "frontends/Application.h"
#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include <algorithm>

using namespace std;

namespace lyx {

namespace {

typedef Translator<string, InsetScriptParams::Type> ScriptTranslator;
typedef Translator<docstring, InsetScriptParams::Type> ScriptTranslatorLoc;

ScriptTranslator const init_scripttranslator()
{
	ScriptTranslator translator("subscript", InsetScriptParams::Subscript);
	translator.addPair("superscript", InsetScriptParams::Superscript);
	return translator;
}


ScriptTranslatorLoc const init_scripttranslator_loc()
{
	ScriptTranslatorLoc translator(_("Subscript"), InsetScriptParams::Subscript);
	translator.addPair(_("Superscript"), InsetScriptParams::Superscript);
	return translator;
}


ScriptTranslator const & scripttranslator()
{
	static ScriptTranslator const translator =
	    init_scripttranslator();
	return translator;
}


ScriptTranslatorLoc const & scripttranslator_loc()
{
	static ScriptTranslatorLoc const translator =
	    init_scripttranslator_loc();
	return translator;
}

} // anon


InsetScriptParams::InsetScriptParams()
	: type(Subscript)
{}


void InsetScriptParams::write(ostream & os) const
{
	string const label = scripttranslator().find(type);
	os << "script " << label << "\n";
}


void InsetScriptParams::read(Lexer & lex)
{
	string label;
	lex >> label;
	if (lex)
		type = scripttranslator().find(label);
}


int InsetScriptParams::shift(FontInfo const & font) const
{
	frontend::FontMetrics const & fm = theFontMetrics(font);
	switch (type) {
	case Subscript:
		return fm.maxAscent() / 3;
	case Superscript:
		return -fm.maxAscent() / 2;
	}
	// shut up compiler
	return 0;
}


/////////////////////////////////////////////////////////////////////
//
// InsetScript
//
/////////////////////////////////////////////////////////////////////

InsetScript::InsetScript(Buffer * buf, InsetScriptParams const & params)
	: InsetText(buf, InsetText::PlainLayout), params_(params)
{
	setDrawFrame(false);
}


InsetScript::InsetScript(Buffer * buf, string const & label)
	: InsetText(buf)
{
	setDrawFrame(false);
	params_.type = scripttranslator().find(label);
}


InsetScript::~InsetScript()
{
}


docstring InsetScript::layoutName() const
{
	return from_ascii("Script:" + scripttranslator().find(params_.type));
}


Inset::DisplayType InsetScript::display() const
{
	return Inline;
}


void InsetScript::metrics(MetricsInfo & mi, Dimension & dim) const
{
	int const shift = params_.shift(mi.base.font);
	ScriptChanger dummy(mi.base);
	InsetText::metrics(mi, dim);
	dim.asc -= shift;
	dim.des += shift;
}


void InsetScript::draw(PainterInfo & pi, int x, int y) const
{
	int const shift = params_.shift(pi.base.font);
	ScriptChanger dummy(pi.base);
	InsetText::draw(pi, x, y + shift);
}


void InsetScript::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const
{
	Font const font = bv.textMetrics(&text()).displayFont(sl.pit(), sl.pos());
	int const shift = params_.shift(font.fontInfo());
	InsetText::cursorPos(bv, sl, boundary, x, y);
	y += shift;
}


void InsetScript::write(ostream & os) const
{
	params_.write(os);
	text().write(os);
}


void InsetScript::read(Lexer & lex)
{
	params_.read(lex);
	InsetText::read(lex);
}


void InsetScript::edit(Cursor & cur, bool front, EntryDirection entry_from)
{
	cur.push(*this);
	InsetText::edit(cur, front, entry_from);
}


Inset * InsetScript::editXY(Cursor & cur, int x, int y)
{
	cur.push(*this);
	return InsetText::editXY(cur, x, y);
}

void InsetScript::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {
	case LFUN_INSET_MODIFY:
		cur.recordUndoInset(this);
		string2params(to_utf8(cmd.argument()), params_);
		break;
	default:
		InsetText::doDispatch(cur, cmd);
		break;
	}
}


bool InsetScript::insetAllowed(InsetCode code) const
{
	switch (code) {
	// code that is not allowed in a script
	case BIBITEM_CODE:
	case BIBTEX_CODE:
	case BOX_CODE:
	case BRANCH_CODE:
	case CAPTION_CODE:
	case COLLAPSABLE_CODE:
	case FLOAT_CODE:
	case FLOAT_LIST_CODE:
	case FOOT_CODE:
	case INCLUDE_CODE:
	case INDEX_PRINT_CODE:
	case LISTINGS_CODE:
	case MARGIN_CODE:
	case MATH_MACRO_CODE:
	case MATHMACRO_CODE:
	case NEWLINE_CODE:
	case NEWPAGE_CODE:
	case NOMENCL_PRINT_CODE:
	case QUOTE_CODE:
	case PREVIEW_CODE:
	case TABULAR_CODE:
	case TOC_CODE:
	case WRAP_CODE:
		return false;
	default:
		return InsetText::insetAllowed(code);
	}
}

bool InsetScript::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action()) {
	case LFUN_MATH_DISPLAY:
	case LFUN_BOX_INSERT:
	case LFUN_BRANCH_INSERT:
	case LFUN_CAPTION_INSERT:
	case LFUN_FLOAT_INSERT:
	case LFUN_FLOAT_LIST_INSERT:
	case LFUN_FLOAT_WIDE_INSERT:
	case LFUN_FOOTNOTE_INSERT:
	case LFUN_INDEX_PRINT:
	case LFUN_LISTING_INSERT:
	case LFUN_MARGINALNOTE_INSERT:
	case LFUN_NEWLINE_INSERT:
	case LFUN_NEWPAGE_INSERT:
	case LFUN_NOMENCL_PRINT:
	case LFUN_PREVIEW_INSERT:
	case LFUN_QUOTE_INSERT:
	case LFUN_TABULAR_INSERT:
	case LFUN_WRAP_INSERT:
		flag.setEnabled(false);
		return true;
	case LFUN_INSET_MODIFY:
		flag.setEnabled(true);
		return true;
	case LFUN_COMMAND_SEQUENCE: {
		// argument contains ';'-terminated commands
		string arg = to_utf8(cmd.argument());
		// prevent insertion of display math formulas like AMS align
		while (!arg.empty()) {
			string first;
			arg = support::split(arg, first, ';');
			FuncRequest func(lyxaction.lookupFunc(first));
			if (func.action() == LFUN_MATH_MUTATE) {
				flag.setEnabled(false);
				return true;
			}
		}
		break;
	}
	default:
		break;
	}
	return InsetText::getStatus(cur, cmd, flag);
}


docstring InsetScript::toolTip(BufferView const &, int, int) const
{
	OutputParams rp(&buffer().params().encoding());
	odocstringstream ods;
	InsetText::plaintext(ods, rp, 200);
	docstring content_tip = ods.str();
	// shorten it if necessary
	support::truncateWithEllipsis(content_tip, 200);
	docstring res = scripttranslator_loc().find(params_.type);
	if (!content_tip.empty())
		res += from_ascii(": ") + content_tip;
	return res;
}


int InsetScript::plaintext(odocstringstream & os,
        OutputParams const & runparams, size_t max_length) const
{
	odocstringstream oss;
	InsetText::plaintext(oss, runparams, max_length);
	docstring const text = oss.str();
	switch (params_.type) {
	case InsetScriptParams::Subscript:
		if (text.size() == 1) {
			char_type const c = support::subscript(text[0]);
			if (c != text[0]) {
				os.put(c);
				return 0;
			}
		}
		os << '[' << buffer().B_("subscript") << ':';
		break;
	case InsetScriptParams::Superscript:
		if (text.size() == 1) {
			char_type const c = support::superscript(text[0]);
			if (c != text[0]) {
				os.put(c);
				return 0;
			}
		}
		os << '[' << buffer().B_("superscript") << ':';
		break;
	}
	InsetText::plaintext(os, runparams, max_length);
	os << ']';

	return PLAINTEXT_NEWLINE;
}


int InsetScript::docbook(odocstream & os, OutputParams const & runparams) const
{
	docstring cmdname;
	switch (params_.type) {
	case InsetScriptParams::Subscript:
		cmdname = from_ascii("subscript");
		break;
	case InsetScriptParams::Superscript:
		cmdname = from_ascii("superscript");
		break;
	}
	os << '<' + cmdname + '>';
	int const i = InsetText::docbook(os, runparams);
	os << "</" + cmdname + '>';

	return i;
}


string InsetScript::contextMenuName() const
{
	return "context-script";
}


string InsetScript::params2string(InsetScriptParams const & params)
{
	ostringstream data;
	data << "script ";
	params.write(data);
	return data.str();
}


void InsetScript::string2params(string const & in, InsetScriptParams & params)
{
	params = InsetScriptParams();

	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex;
	lex.setStream(data);
	lex.setContext("InsetScript::string2params");
	lex >> "script" >> "script";

	params.read(lex);
}


} // namespace lyx
