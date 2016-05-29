/**
 * \file InsetNewpage.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetNewpage.h"

#include "Cursor.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "output_xhtml.h"
#include "Text.h"
#include "TextMetrics.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/docstream.h"
#include "support/gettext.h"

using namespace std;


namespace lyx {

	InsetNewpage::InsetNewpage() : Inset(0)
{}


InsetNewpage::InsetNewpage(InsetNewpageParams const & params)
	: Inset(0), params_(params)
{}


void InsetNewpageParams::write(ostream & os) const
{
	switch (kind) {
	case InsetNewpageParams::NEWPAGE:
		os << "newpage";
		break;
	case InsetNewpageParams::PAGEBREAK:
		os <<  "pagebreak";
		break;
	case InsetNewpageParams::CLEARPAGE:
		os <<  "clearpage";
		break;
	case InsetNewpageParams::CLEARDOUBLEPAGE:
		os <<  "cleardoublepage";
		break;
	}
}


void InsetNewpageParams::read(Lexer & lex)
{
	lex.setContext("InsetNewpageParams::read");
	string token;
	lex >> token;

	if (token == "newpage")
		kind = InsetNewpageParams::NEWPAGE;
	else if (token == "pagebreak")
		kind = InsetNewpageParams::PAGEBREAK;
	else if (token == "clearpage")
		kind = InsetNewpageParams::CLEARPAGE;
	else if (token == "cleardoublepage")
		kind = InsetNewpageParams::CLEARDOUBLEPAGE;
	else
		lex.printError("Unknown kind");
}


void InsetNewpage::write(ostream & os) const
{
	os << "Newpage ";
	params_.write(os);
}


void InsetNewpage::read(Lexer & lex)
{
	params_.read(lex);
	lex >> "\\end_inset";
}


void InsetNewpage::metrics(MetricsInfo & mi, Dimension & dim) const
{
	dim.asc = defaultRowHeight();
	dim.des = defaultRowHeight();
	dim.wid = mi.base.textwidth;
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetNewpage::draw(PainterInfo & pi, int x, int y) const
{
	using frontend::Painter;

	FontInfo font;
	font.setColor(ColorName());
	font.decSize();

	Dimension const dim = dimension(*pi.base.bv);

	int w = 0;
	int a = 0;
	int d = 0;
	theFontMetrics(font).rectText(insetLabel(), w, a, d);

	int const text_start = int(x + (dim.wid - w) / 2);
	int const text_end = text_start + w;

	pi.pain.rectText(text_start, y + d, insetLabel(), font,
		Color_none, Color_none);

	pi.pain.line(x, y, text_start, y,
		   ColorName(), Painter::line_onoffdash);
	pi.pain.line(text_end, y, int(x + dim.wid), y,
		   ColorName(), Painter::line_onoffdash);
}


void InsetNewpage::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		InsetNewpageParams params;
		cur.recordUndo();
		string2params(to_utf8(cmd.argument()), params);
		params_.kind = params.kind;
		break;
	}

	default:
		Inset::doDispatch(cur, cmd);
		break;
	}
}


bool InsetNewpage::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action()) {
	// we handle these
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "newpage") {
			InsetNewpageParams params;
			string2params(to_utf8(cmd.argument()), params);
			status.setOnOff(params_.kind == params.kind);
		} 
		status.setEnabled(true);
		return true;
	default:
		return Inset::getStatus(cur, cmd, status);
	}
}


docstring InsetNewpage::insetLabel() const
{
	switch (params_.kind) {
		case InsetNewpageParams::NEWPAGE:
			return _("New Page");
			break;
		case InsetNewpageParams::PAGEBREAK:
			return _("Page Break");
			break;
		case InsetNewpageParams::CLEARPAGE:
			return _("Clear Page");
			break;
		case InsetNewpageParams::CLEARDOUBLEPAGE:
			return _("Clear Double Page");
			break;
		default:
			return _("New Page");
			break;
	}
}


ColorCode InsetNewpage::ColorName() const
{
	switch (params_.kind) {
		case InsetNewpageParams::PAGEBREAK:
			return Color_pagebreak;
			break;
		case InsetNewpageParams::NEWPAGE:
		case InsetNewpageParams::CLEARPAGE:
		case InsetNewpageParams::CLEARDOUBLEPAGE:
			return Color_newpage;
			break;
	}
	// not really useful, but to avoids gcc complaints
	return Color_newpage;
}


void InsetNewpage::latex(otexstream & os, OutputParams const & runparams) const
{
	switch (params_.kind) {
		case InsetNewpageParams::NEWPAGE:
			os << "\\newpage{}";
			break;
		case InsetNewpageParams::PAGEBREAK:
			if (runparams.moving_arg)
				os << "\\protect";
			os << "\\pagebreak{}";
			break;
		case InsetNewpageParams::CLEARPAGE:
			os << "\\clearpage{}";
			break;
		case InsetNewpageParams::CLEARDOUBLEPAGE:
			os << "\\cleardoublepage{}";
			break;
		default:
			os << "\\newpage{}";
			break;
	}
}


int InsetNewpage::plaintext(odocstringstream & os,
        OutputParams const &, size_t) const
{
	os << '\n';
	return PLAINTEXT_NEWLINE;
}


int InsetNewpage::docbook(odocstream & os, OutputParams const &) const
{
	os << '\n';
	return 0;
}


docstring InsetNewpage::xhtml(XHTMLStream & xs, OutputParams const &) const
{
	xs << html::CompTag("br");
	return docstring();
}


string InsetNewpage::contextMenuName() const
{
	return "context-newpage";
}


void InsetNewpage::string2params(string const & in, InsetNewpageParams & params)
{
	params = InsetNewpageParams();
	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex;
	lex.setStream(data);

	string name;
	lex >> name;
	if (!lex || name != "newpage") {
		LYXERR0("Expected arg 2 to be \"wrap\" in " << in);
		return;
	}

	params.read(lex);
}


string InsetNewpage::params2string(InsetNewpageParams const & params)
{
	ostringstream data;
	data << "newpage" << ' ';
	params.write(data);
	return data.str();
}


} // namespace lyx
