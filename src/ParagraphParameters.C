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
#include "ParameterStruct.h"
#include "tex-strings.h"

#include "frontends/LyXView.h"

#include "support/lstrings.h"
#include "support/std_sstream.h"

using lyx::support::rtrim;

using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::string;


// Initialize static member var.
ShareContainer<ParameterStruct> ParagraphParameters::container;

ParagraphParameters::ParagraphParameters()
{
	ParameterStruct tmp;
	set_from_struct(tmp);
}


void ParagraphParameters::clear()
{
	ParameterStruct tmp(*param);
	tmp.spacing.set(Spacing::Default);
	tmp.align = LYX_ALIGN_LAYOUT;
	tmp.depth = 0;
	tmp.noindent = false;
	tmp.labelstring.erase();
	tmp.labelwidthstring.erase();
	tmp.start_of_appendix = false;
	set_from_struct(tmp);
}


ParagraphParameters::depth_type ParagraphParameters::depth() const
{
	return param->depth;
}


bool ParagraphParameters::sameLayout(ParagraphParameters const & pp) const
{
	return param->align == pp.param->align &&
		param->spacing == pp.param->spacing &&
		param->noindent == pp.param->noindent &&
		param->depth == pp.param->depth;
}


void ParagraphParameters::set_from_struct(ParameterStruct const & ps)
{
	// get new param from container with tmp as template
	param = container.get(ps);
}


Spacing const & ParagraphParameters::spacing() const
{
	return param->spacing;
}


void ParagraphParameters::spacing(Spacing const & s)
{
	ParameterStruct tmp(*param);
	tmp.spacing = s;
	set_from_struct(tmp);
}


bool ParagraphParameters::noindent() const
{
	return param->noindent;
}


void ParagraphParameters::noindent(bool ni)
{
	ParameterStruct tmp(*param);
	tmp.noindent = ni;
	set_from_struct(tmp);
}


LyXAlignment ParagraphParameters::align() const
{
	return param->align;
}


void ParagraphParameters::align(LyXAlignment la)
{
	ParameterStruct tmp(*param);
	tmp.align = la;
	set_from_struct(tmp);
}


void ParagraphParameters::depth(depth_type d)
{
	ParameterStruct tmp(*param);
	tmp.depth = d;
	set_from_struct(tmp);
}


bool ParagraphParameters::startOfAppendix() const
{
	return param->start_of_appendix;
}


void ParagraphParameters::startOfAppendix(bool soa)
{
	ParameterStruct tmp(*param);
	tmp.start_of_appendix = soa;
	set_from_struct(tmp);
}


bool ParagraphParameters::appendix() const
{
	return param->appendix;
}


void ParagraphParameters::appendix(bool a)
{
	ParameterStruct tmp(*param);
	tmp.appendix = a;
	set_from_struct(tmp);
}


string const & ParagraphParameters::labelString() const
{
	return param->labelstring;
}


void ParagraphParameters::labelString(string const & ls)
{
	ParameterStruct tmp(*param);
	tmp.labelstring = ls;
	set_from_struct(tmp);
}


string const & ParagraphParameters::labelWidthString() const
{
	return param->labelwidthstring;
}


void ParagraphParameters::labelWidthString(string const & lws)
{
	ParameterStruct tmp(*param);
	tmp.labelwidthstring = lws;
	set_from_struct(tmp);
}


LyXLength const & ParagraphParameters::leftIndent() const
{
	return param->leftindent;
}


void ParagraphParameters::leftIndent(LyXLength const & li)
{
	ParameterStruct tmp(*param);
	tmp.leftindent = li;
	set_from_struct(tmp);
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
			int tmpret = lex.findToken(string_align);
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
	// Maybe the paragraph has special spacing
	spacing().writeFile(os, true);

	// The labelwidth string used in lists.
	if (!labelWidthString().empty())
		os << "\\labelwidthstring "
		   << labelWidthString() << '\n';

	// Start of appendix?
	if (startOfAppendix())
		os << "\\start_of_appendix ";

	// Noindent?
	if (noindent())
		os << "\\noindent ";

	// Do we have a manual left indent?
	if (!leftIndent().zero())
		os << "\\leftindent " << leftIndent().asString()
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
		os << "\\align " << string_align[h] << ' ';
	}
}


void setParagraphParams(BufferView & bv, string const & data)
{
	istringstream is(data);
	LyXLex lex(0,0);
	lex.setStream(is);

	ParagraphParameters params;
	params.read(lex);

	LyXText * text = bv.getLyXText();
	text->setParagraph(
			   params.spacing(),
			   params.align(),
			   params.labelWidthString(),
			   params.noindent());

	bv.update();
	bv.owner()->message(_("Paragraph layout set"));
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

	/// is paragraph in inset
	os << "\\ininset " << (par.inInset()?1:0) << '\n';

	data = os.str();
}
