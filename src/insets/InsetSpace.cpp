/**
 * \file InsetSpace.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Jean-Marc Lasgouttes
 * \author Lars Gullik Bjønnes
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetSpace.h"

#include "debug.h"
#include "Color.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "OutputParams.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"


namespace lyx {

using std::string;
using std::max;
using std::auto_ptr;
using std::ostream;


InsetSpace::InsetSpace()
{}


InsetSpace::InsetSpace(Kind k)
	: kind_(k)
{}


InsetSpace::Kind InsetSpace::kind() const
{
	return kind_;
}


bool InsetSpace::metrics(MetricsInfo & mi, Dimension & dim) const
{
	frontend::FontMetrics const & fm =
		theFontMetrics(mi.base.font);
	dim.asc = fm.maxAscent();
	dim.des = fm.maxDescent();

	switch (kind_) {
		case THIN:
		case NEGTHIN:
			dim.wid = fm.width(char_type('M')) / 6;
			break;
		case PROTECTED:
		case NORMAL:
			dim.wid = fm.width(char_type(' '));
			break;
		case QUAD:
			dim.wid = fm.width(char_type('M'));
			break;
		case QQUAD:
			dim.wid = 2 * fm.width(char_type('M'));
			break;
		case ENSPACE:
		case ENSKIP:
			dim.wid = fm.width(char_type('M')) / 2;
			break;
	}
	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


void InsetSpace::draw(PainterInfo & pi, int x, int y) const
{
	int const w = width();
	int const h = theFontMetrics(pi.base.font)
		.ascent('x');
	int xp[4], yp[4];

	xp[0] = x;
	yp[0] = y - max(h / 4, 1);
	if (kind_ == NORMAL || kind_ == PROTECTED) {
		xp[1] = x;     yp[1] = y;
		xp[2] = x + w; yp[2] = y;
	} else {
		xp[1] = x;     yp[1] = y + max(h / 4, 1);
		xp[2] = x + w; yp[2] = y + max(h / 4, 1);
	}
	xp[3] = x + w;
	yp[3] = y - max(h / 4, 1);

	if (kind_ == PROTECTED || kind_ == ENSPACE || kind_ == NEGTHIN)
		pi.pain.lines(xp, yp, 4, Color::latex);
	else
		pi.pain.lines(xp, yp, 4, Color::special);
}


void InsetSpace::write(Buffer const &, ostream & os) const
{
	string command;
	switch (kind_) {
	case NORMAL:
		command = "\\space{}";
		break;
	case PROTECTED:
		command = "~";
		break;
	case THIN:
		command = "\\thinspace{}";
		break;
	case QUAD:
		command = "\\quad{}";
		break;
	case QQUAD:
		command = "\\qquad{}";
		break;
	case ENSPACE:
		command = "\\enspace{}";
		break;
	case ENSKIP:
		command = "\\enskip{}";
		break;
	case NEGTHIN:
		command = "\\negthinspace{}";
		break;
	}
	os << "\\InsetSpace " << command << "\n";
}


void InsetSpace::read(Buffer const &, Lexer & lex)
{
	lex.next();
	string const command = lex.getString();

	if (command == "\\space{}")
		kind_ = NORMAL;
	else if (command == "~")
		kind_ = PROTECTED;
	else if (command == "\\thinspace{}")
		kind_ = THIN;
	else if (command == "\\quad{}")
		kind_ = QUAD;
	else if (command == "\\qquad{}")
		kind_ = QQUAD;
	else if (command == "\\enspace{}")
		kind_ = ENSPACE;
	else if (command == "\\enskip{}")
		kind_ = ENSKIP;
	else if (command == "\\negthinspace{}")
		kind_ = NEGTHIN;
	else
		lex.printError("InsetSpace: Unknown kind: `$$Token'");
}


int InsetSpace::latex(Buffer const &, odocstream & os,
		      OutputParams const & runparams) const
{
	switch (kind_) {
	case NORMAL:
		os << (runparams.free_spacing ? " " : "\\ ");
		break;
	case PROTECTED:
		os << (runparams.free_spacing ? ' ' : '~');
		break;
	case THIN:
		os << (runparams.free_spacing ? " " : "\\,");
		break;
	case QUAD:
		os << (runparams.free_spacing ? " " : "\\quad{}");
		break;
	case QQUAD:
		os << (runparams.free_spacing ? " " : "\\qquad{}");
		break;
	case ENSPACE:
		os << (runparams.free_spacing ? " " : "\\enspace{}");
		break;
	case ENSKIP:
		os << (runparams.free_spacing ? " " : "\\enskip{}");
		break;
	case NEGTHIN:
		os << (runparams.free_spacing ? " " : "\\negthinspace{}");
		break;
	}
	return 0;
}


int InsetSpace::plaintext(Buffer const &, odocstream & os,
			  OutputParams const &) const
{
	os << ' ';
	return 1;
}


int InsetSpace::docbook(Buffer const &, odocstream & os,
			OutputParams const &) const
{
	switch (kind_) {
	case NORMAL:
	case QUAD:
	case QQUAD:
	case ENSKIP:
		os << " ";
		break;
	case PROTECTED:
	case ENSPACE:
	case THIN:
	case NEGTHIN:
		os << "&nbsp;";
		break;
	}
	return 0;
}


void InsetSpace::textString(Buffer const & buf, odocstream & os) const
{
	plaintext(buf, os, OutputParams(0));
}


auto_ptr<Inset> InsetSpace::doClone() const
{
	return auto_ptr<Inset>(new InsetSpace(kind_));
}


bool InsetSpace::isChar() const
{
	return true;
}

bool InsetSpace::isLetter() const
{
	return false;
}

bool InsetSpace::isSpace() const
{
	return true;
}


} // namespace lyx
