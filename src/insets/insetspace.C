/**
 * \file insetspace.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Jean-Marc Lasgouttes
 * \author Lars Gullik Bjønnes
 * \author Juergen Spitzmueller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetspace.h"

#include "debug.h"
#include "dimension.h"
#include "LaTeXFeatures.h"
#include "latexrunparams.h"
#include "BufferView.h"
#include "frontends/Painter.h"
#include "frontends/font_metrics.h"
#include "lyxlex.h"
#include "lyxfont.h"
#include "metricsinfo.h"

using std::ostream;
using std::max;
using std::auto_ptr;


InsetSpace::InsetSpace()
{}


InsetSpace::InsetSpace(Kind k)
	: kind_(k)
{}


InsetSpace::Kind InsetSpace::kind() const
{
	return kind_;
}


void InsetSpace::metrics(MetricsInfo & mi, Dimension & dim) const
{
	LyXFont & font = mi.base.font;
	dim.asc = font_metrics::maxAscent(font);
	dim.des = font_metrics::maxDescent(font);

	switch (kind_) {
		case THIN:
		case NEGTHIN:
			dim.wid = font_metrics::width("x", font) / 3;
			break;
		case PROTECTED:
		case NORMAL:
			dim.wid = font_metrics::width("x", font);
			break;
		case QUAD:
			dim.wid = 20;
			break;
		case QQUAD:
			dim.wid = 40;
			break;
		case ENSPACE:
		case ENSKIP:
			dim.wid = 10;
			break;
	}
	dim_ = dim;
}


void InsetSpace::draw(PainterInfo & pi, int x, int y) const
{
	int const w = width();
	int const h = font_metrics::ascent('x', pi.base.font);
	int xp[4], yp[4];

	xp[0] = x;
	yp[0] = y - max(h / 4, 1);
	if (kind_ == NORMAL) {
		xp[1] = x;     yp[1] = y;
		xp[2] = x + w; yp[2] = y;
	} else {
		xp[1] = x;     yp[1] = y + max(h / 4, 1);
		xp[2] = x + w; yp[2] = y + max(h / 4, 1);
	}
	xp[3] = x + w;
	yp[3] = y - max(h / 4, 1);

	if (kind_ == PROTECTED || kind_ == ENSPACE || kind_ == NEGTHIN)
		pi.pain.lines(xp, yp, 4, LColor::latex);
	else
		pi.pain.lines(xp, yp, 4, LColor::special);
}


void InsetSpace::write(Buffer const &, ostream & os) const
{
	string command;
	switch (kind_) {
	case NORMAL:
		command = "\\space";
		break;
	case PROTECTED:
		command = "~";
		break;
	case THIN:
		command = "\\,";
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


// This function will not be necessary when lyx3
void InsetSpace::read(Buffer const &, LyXLex & lex)
{
	lex.nextToken();
	string const command = lex.getString();

	if (command == "\\space")
		kind_ = NORMAL;
	else if (command == "~")
		kind_ = PROTECTED;
	else if (command == "\\,")
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


int InsetSpace::latex(Buffer const &, ostream & os,
		      LatexRunParams const & runparams) const
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


int InsetSpace::ascii(Buffer const &, ostream & os, int) const
{
	switch (kind_) {
	case NORMAL:
	case PROTECTED:
	case THIN:
	case QUAD:
	case QQUAD:
	case ENSPACE:
	case ENSKIP:
	case NEGTHIN:
		os << ' ';
		break;
	}
	return 0;
}


int InsetSpace::linuxdoc(Buffer const &, ostream & os) const
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


int InsetSpace::docbook(Buffer const &, ostream & os, bool) const
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


auto_ptr<InsetBase>  InsetSpace::clone() const
{
	return auto_ptr<InsetBase>(new InsetSpace(kind_));
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

bool InsetSpace::isLineSeparator() const
{
	return false;
}
