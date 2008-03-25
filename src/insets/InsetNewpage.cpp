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

#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Text.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "TextMetrics.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/docstream.h"
#include "support/gettext.h"

using namespace std;


namespace lyx {

InsetNewpage::InsetNewpage()
{}


InsetNewpage::InsetNewpage(InsetNewpageParams par)
{
	params_.kind = par.kind;
}

void InsetNewpageParams::write(ostream & os) const
{
	string command;
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
	lex.next();
	string const command = lex.getString();

	if (command == "newpage")
		kind = InsetNewpageParams::NEWPAGE;
	else if (command == "pagebreak")
		kind = InsetNewpageParams::PAGEBREAK;
	else if (command == "clearpage")
		kind = InsetNewpageParams::CLEARPAGE;
	else if (command == "cleardoublepage")
		kind = InsetNewpageParams::CLEARDOUBLEPAGE;
	else
		lex.printError("InsetNewpage: Unknown kind: `$$Token'");

	string token;
	lex >> token;
	if (!lex)
		return;
	if (token != "\\end_inset")
		lex.printError("Missing \\end_inset at this point. "
			       "Read: `$$Token'");
}


void InsetNewpage::write(ostream & os) const
{
	os << "Newpage ";
	params_.write(os);
}


void InsetNewpage::read(Lexer & lex)
{
	params_.read(lex);
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
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetNewpageParams params;
		InsetNewpageMailer::string2params(to_utf8(cmd.argument()), params);
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
	switch (cmd.action) {
	// we handle these
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "newpage") {
			InsetNewpageParams params;
			InsetNewpageMailer::string2params(to_utf8(cmd.argument()), params);
			status.setOnOff(params_.kind == params.kind);
		} else
			status.enabled(true);
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
		case InsetNewpageParams::NEWPAGE:
			return Color_newpage;
			break;
		case InsetNewpageParams::PAGEBREAK:
			return Color_pagebreak;
			break;
		case InsetNewpageParams::CLEARPAGE:
			return Color_newpage;
			break;
		case InsetNewpageParams::CLEARDOUBLEPAGE:
			return Color_newpage;
			break;
		default:
			return Color_newpage;
			break;
	}
}


int InsetNewpage::latex(odocstream & os, OutputParams const &) const
{
	switch (params_.kind) {
		case InsetNewpageParams::NEWPAGE:
			os << "\\newpage{}";
			break;
		case InsetNewpageParams::PAGEBREAK:
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
	return 0;
}


int InsetNewpage::plaintext(odocstream & os, OutputParams const &) const
{
	os << '\n';
	return PLAINTEXT_NEWLINE;
}


int InsetNewpage::docbook(odocstream & os, OutputParams const &) const
{
	os << '\n';
	return 0;
}


docstring InsetNewpage::contextMenu(BufferView const &, int, int) const
{
	return from_ascii("context-newpage");
}


string const InsetNewpageMailer::name_ = "newpage";


InsetNewpageMailer::InsetNewpageMailer(InsetNewpage & inset)
	: inset_(inset)
{}


string const InsetNewpageMailer::inset2string(Buffer const &) const
{
	return params2string(inset_.params());
}


void InsetNewpageMailer::string2params(string const & in, InsetNewpageParams & params)
{
	params = InsetNewpageParams();
	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex(0,0);
	lex.setStream(data);

	string name;
	lex >> name;
	if (!lex || name != name_)
		return print_mailer_error("InsetNewpageMailer", in, 1, name_);

	params.read(lex);
}


string const InsetNewpageMailer::params2string(InsetNewpageParams const & params)
{
	ostringstream data;
	data << name_ << ' ';
	params.write(data);
	return data.str();
}


} // namespace lyx
