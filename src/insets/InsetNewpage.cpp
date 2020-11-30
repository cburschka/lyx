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
#include "xml.h"
#include "texstream.h"
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

	InsetNewpage::InsetNewpage() : Inset(nullptr)
{}


InsetNewpage::InsetNewpage(InsetNewpageParams const & params)
	: Inset(nullptr), params_(params)
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
	case InsetNewpageParams::NOPAGEBREAK:
		os <<  "nopagebreak";
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
	else if (token == "nopagebreak")
		kind = InsetNewpageParams::NOPAGEBREAK;
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
	if (params_.kind == InsetNewpageParams::NOPAGEBREAK) {
		frontend::FontMetrics const & fm = theFontMetrics(mi.base.font);
		dim.asc = fm.maxAscent();
	        dim.des = fm.maxDescent();
	        dim.wid = 3 * fm.width('n');
		return;
	}

	dim.asc = defaultRowHeight();
	dim.des = defaultRowHeight();
	dim.wid = mi.base.textwidth;
}


void InsetNewpage::draw(PainterInfo & pi, int x, int y) const
{
	if (params_.kind == InsetNewpageParams::NOPAGEBREAK) {

	        FontInfo font;
	        font.setColor(ColorName());

	        frontend::FontMetrics const & fm = theFontMetrics(pi.base.font);
	        int const wid = 3 * fm.width('n');
	        int const asc = fm.maxAscent();

		int xp[3];
	        int yp[3];

		//left side arrow
		yp[0] = int(y - 0.875 * asc * 0.75);
		yp[1] = int(y - 0.500 * asc * 0.75);
		yp[2] = int(y - 0.125 * asc * 0.75);
		xp[0] = int(x + wid * 0.25);
		xp[1] = int(x + wid * 0.4); 
		xp[2] = int(x + wid * 0.25);
		pi.pain.lines(xp, yp, 3, ColorName());

		yp[0] = yp[1] = int(y - 0.500 * asc * 0.75);
		xp[0] = int(x + wid * 0.03);
		xp[1] = int(x + wid * 0.4); 
		pi.pain.lines(xp, yp, 2, ColorName());

		//right side arrow
		yp[0] = int(y - 0.875 * asc * 0.75);
		yp[1] = int(y - 0.500 * asc * 0.75);
		yp[2] = int(y - 0.125 * asc * 0.75);
		xp[0] = int(x + wid * 0.75);
		xp[1] = int(x + wid * 0.6); 
		xp[2] = int(x + wid * 0.75);
		pi.pain.lines(xp, yp, 3, ColorName());

		yp[0] = yp[1] = int(y - 0.500 * asc * 0.75);
		xp[0] = int(x + wid * 0.97);
		xp[1] = int(x + wid * 0.6); 
		pi.pain.lines(xp, yp, 2, ColorName());

		//mid-rule
		xp[0] = xp[1] = int(x + wid * 0.5);
		yp[0] = int(y - 0.875 * asc * 0.75);
		yp[1] = int(y - 0.125 * asc * 0.75);
		pi.pain.lines(xp, yp, 2, ColorName());
		return;
	}

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
		case InsetNewpageParams::PAGEBREAK:
			return _("Page Break");
		case InsetNewpageParams::CLEARPAGE:
			return _("Clear Page");
		case InsetNewpageParams::CLEARDOUBLEPAGE:
			return _("Clear Double Page");
		case InsetNewpageParams::NOPAGEBREAK:
			return _("No Page Break");
		default:
			return _("New Page");
	}
}


ColorCode InsetNewpage::ColorName() const
{
	switch (params_.kind) {
		case InsetNewpageParams::PAGEBREAK:
		case InsetNewpageParams::NOPAGEBREAK:
			return Color_pagebreak;
		case InsetNewpageParams::NEWPAGE:
		case InsetNewpageParams::CLEARPAGE:
		case InsetNewpageParams::CLEARDOUBLEPAGE:
			return Color_newpage;
	}
	// not really useful, but to avoids gcc complaints
	return Color_newpage;
}


void InsetNewpage::latex(otexstream & os, OutputParams const & runparams) const
{
	if (runparams.inDeletedInset) {
		os << "\\mbox{}\\\\\\makebox[\\columnwidth]{\\dotfill\\ "
		   << insetLabel() << "\\ \\dotfill}";
	} else {
		switch (params_.kind) {
		case InsetNewpageParams::NEWPAGE:
			os << "\\newpage" << termcmd;
			break;
		case InsetNewpageParams::PAGEBREAK:
			if (runparams.moving_arg)
				os << "\\protect";
			os << "\\pagebreak" << termcmd;
			break;
		case InsetNewpageParams::CLEARPAGE:
			os << "\\clearpage" << termcmd;
			break;
		case InsetNewpageParams::CLEARDOUBLEPAGE:
			os << "\\cleardoublepage" << termcmd;
			break;
		case InsetNewpageParams::NOPAGEBREAK:
			os << "\\nopagebreak" << termcmd;
			break;
		default:
			os << "\\newpage" << termcmd;
			break;
		}
	}
}


int InsetNewpage::plaintext(odocstringstream & os,
        OutputParams const &, size_t) const
{
	if (params_.kind ==  InsetNewpageParams::NOPAGEBREAK)
		return 0;
	os << '\n';
	return PLAINTEXT_NEWLINE;
}


void InsetNewpage::docbook(XMLStream & os, OutputParams const &) const
{
	if (params_.kind !=  InsetNewpageParams::NOPAGEBREAK)
		os << xml::CR();
}


docstring InsetNewpage::xhtml(XMLStream & xs, OutputParams const &) const
{
	if (params_.kind !=  InsetNewpageParams::NOPAGEBREAK)
		xs << xml::CompTag("br");
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
