/**
 * \file ParagraphParameters.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ParagraphParameters.h"

#include "buffer.h"
#include "BufferView.h"
#include "gettext.h"
#include "lyxlayout.h"
#include "lyxlex.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "tex-strings.h"

#include "frontends/LyXView.h"

#include "support/lstrings.h"

#include <sstream>

using lyx::support::rtrim;

using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::string;

// anonym namespace
namespace {
int findToken(char const * const str[], string const search_token)
{
	int i = 0;

	if (search_token != "default") {
		while (str[i][0] && str[i] != search_token) {
			++i;
		}
		if (!str[i][0]) {
			i = -1;
		}
	}

	return i;
}

}


ParagraphParameters::ParagraphParameters()
	: noindent_(false),
		start_of_appendix_(false), appendix_(false),
	  align_(LYX_ALIGN_LAYOUT), depth_(0)
{}


void ParagraphParameters::clear()
{
	operator=(ParagraphParameters());
}


ParagraphParameters::depth_type ParagraphParameters::depth() const
{
	return depth_;
}


bool ParagraphParameters::sameLayout(ParagraphParameters const & pp) const
{
	return align_ == pp.align_
		&& spacing_ == pp.spacing_
		&& noindent_ == pp.noindent_
		&& depth_ == pp.depth_;
}


Spacing const & ParagraphParameters::spacing() const
{
	return spacing_;
}


void ParagraphParameters::spacing(Spacing const & s)
{
	spacing_ = s;
}


bool ParagraphParameters::noindent() const
{
	return noindent_;
}


void ParagraphParameters::noindent(bool ni)
{
	noindent_ = ni;
}


LyXAlignment ParagraphParameters::align() const
{
	return align_;
}


void ParagraphParameters::align(LyXAlignment la)
{
	align_ = la;
}


void ParagraphParameters::depth(depth_type d)
{
	depth_ = d;
}


bool ParagraphParameters::startOfAppendix() const
{
	return start_of_appendix_;
}


void ParagraphParameters::startOfAppendix(bool soa)
{
	start_of_appendix_ = soa;
}


bool ParagraphParameters::appendix() const
{
	return appendix_;
}


void ParagraphParameters::appendix(bool a)
{
	appendix_ = a;
}


string const & ParagraphParameters::labelString() const
{
	return labelstring_;
}


void ParagraphParameters::labelString(string const & ls)
{
	labelstring_ = ls;
}


string const & ParagraphParameters::labelWidthString() const
{
	return labelwidthstring_;
}


void ParagraphParameters::labelWidthString(string const & lws)
{
	labelwidthstring_ = lws;
}


LyXLength const & ParagraphParameters::leftIndent() const
{
	return leftindent_;
}


void ParagraphParameters::leftIndent(LyXLength const & li)
{
	leftindent_ = li;
}


void ParagraphParameters::read(LyXLex & lex)
{
	while (lex.isOK()) {
		lex.nextToken();
		string const token = lex.getString();

		if (token.empty())
			continue;

		if (token[0] != '\\') {
			lex.pushToken(token);
			break;
		}

		if (token == "\\noindent") {
			noindent(true);
		} else if (token == "\\leftindent") {
			lex.nextToken();
			LyXLength value(lex.getString());
			leftIndent(value);
		} else if (token == "\\start_of_appendix") {
			startOfAppendix(true);
		} else if (token == "\\paragraph_spacing") {
			lex.next();
			string const tmp = rtrim(lex.getString());
			if (tmp == "single") {
				spacing(Spacing(Spacing::Single));
			} else if (tmp == "onehalf") {
				spacing(Spacing(Spacing::Onehalf));
			} else if (tmp == "double") {
				spacing(Spacing(Spacing::Double));
			} else if (tmp == "other") {
				lex.next();
				spacing(Spacing(Spacing::Other,
						 lex.getFloat()));
			} else {
				lex.printError("Unknown spacing token: '$$Token'");
			}
		} else if (token == "\\align") {
			lex.next();
			int tmpret = findToken(string_align, lex.getString());
			if (tmpret == -1)
				++tmpret;
			align(LyXAlignment(1 << tmpret));
		} else if (token == "\\labelwidthstring") {
			lex.eatLine();
			labelWidthString(lex.getString());
		} else {
			lex.pushToken(token);
			break;
		}
	}
}


void ParagraphParameters::write(ostream & os) const
{
	ostringstream oss;

	// Maybe the paragraph has special spacing
	spacing().writeFile(oss, true);

	// The labelwidth string used in lists.
	if (!labelWidthString().empty())
		oss << "\\labelwidthstring "
		   << labelWidthString() << '\n';

	// Start of appendix?
	if (startOfAppendix())
		oss << "\\start_of_appendix ";

	// Noindent?
	if (noindent())
		oss << "\\noindent ";

	// Do we have a manual left indent?
	if (!leftIndent().zero())
		oss << "\\leftindent " << leftIndent().asString()
		   << ' ';

	// Alignment?
	if (align() != LYX_ALIGN_LAYOUT) {
		int h = 0;
		switch (align()) {
		case LYX_ALIGN_LEFT: h = 1; break;
		case LYX_ALIGN_RIGHT: h = 2; break;
		case LYX_ALIGN_CENTER: h = 3; break;
		default: h = 0; break;
		}
		oss << "\\align " << string_align[h] << ' ';
	}
	os << rtrim(oss.str());
}


void params2string(Paragraph const & par, string & data)
{
	// A local copy
	ParagraphParameters params = par.params();

	// This needs to be done separately
	params.labelWidthString(par.getLabelWidthString());

	// Alignment
	LyXLayout_ptr const & layout = par.layout();
	if (params.align() == LYX_ALIGN_LAYOUT)
		params.align(layout->align);

	ostringstream os;
	params.write(os);

	// Is alignment possible
	os << '\n' << "\\alignpossible " << layout->alignpossible << '\n';

	/// set default alignment
	os << "\\aligndefault " << layout->align << '\n';

	/// paragraph is always in inset. This is redundant.
	os << "\\ininset " << 1 << '\n';

	data = os.str();
}


/*
bool operator==(ParagraphParameeters const & ps1,
		ParagraphParameeters const & ps2)
{
	return
		   ps1.spacing == ps2.spacing
		&& ps1.noindent == ps2.noindent
		&& ps1.align == ps2.align
		&& ps1.depth == ps2.depth
		&& ps1.start_of_appendix == ps2.start_of_appendix
		&& ps1.appendix == ps2.appendix
		&& ps1.labelstring == ps2.labelstring
		&& ps1.labelwidthstring == ps2.labelwidthstring
		&& ps1.leftindent == ps2.leftindent;
}
*/
