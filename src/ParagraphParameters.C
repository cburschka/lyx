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
#include "lyxlex.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "tex-strings.h"

#include "frontends/LyXView.h"

#include "support/lstrings.h"

#include "support/std_sstream.h"

using namespace lyx::support;

using std::istringstream;
using std::ostream;
using std::ostringstream;


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
	tmp.line_top = false;
	tmp.line_bottom = false;
	tmp.pagebreak_top = false;
	tmp.pagebreak_bottom = false;
	tmp.added_space_top = VSpace(VSpace::NONE);
	tmp.added_space_bottom = VSpace(VSpace::NONE);
	tmp.spacing.set(Spacing::Default);
	tmp.align = LYX_ALIGN_LAYOUT;
	tmp.depth = 0;
	tmp.noindent = false;
	tmp.labelstring.erase();
	tmp.labelwidthstring.erase();
	tmp.start_of_appendix = false;
	set_from_struct(tmp);
}


bool ParagraphParameters::sameLayout(ParagraphParameters const & pp) const
{
	return param->align == pp.param->align &&
		param->line_bottom == pp.param->line_bottom &&
		param->pagebreak_bottom == pp.param->pagebreak_bottom &&
		param->added_space_bottom == pp.param->added_space_bottom &&

		param->line_top == pp.param->line_top &&
		param->pagebreak_top == pp.param->pagebreak_top &&
		param->added_space_top == pp.param->added_space_top &&
		param->spacing == pp.param->spacing &&
		param->noindent == pp.param->noindent &&
		param->depth == pp.param->depth;
}


void ParagraphParameters::set_from_struct(ParameterStruct const & ps)
{
	// get new param from container with tmp as template
	param = container.get(ps);
}


VSpace const & ParagraphParameters::spaceTop() const
{
	return param->added_space_top;
}


void ParagraphParameters::spaceTop(VSpace const & vs)
{
	ParameterStruct tmp(*param);
	tmp.added_space_top = vs;
	set_from_struct(tmp);
}


VSpace const & ParagraphParameters::spaceBottom() const
{
	return param->added_space_bottom;
}


void ParagraphParameters::spaceBottom(VSpace const & vs)
{
	ParameterStruct tmp(*param);
	tmp.added_space_bottom = vs;
	set_from_struct(tmp);
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


bool ParagraphParameters::lineTop() const
{
	return param->line_top;
}


void ParagraphParameters::lineTop(bool lt)
{
	ParameterStruct tmp(*param);
	tmp.line_top = lt;
	set_from_struct(tmp);
}


bool ParagraphParameters::lineBottom() const
{
	return param->line_bottom;
}


void ParagraphParameters::lineBottom(bool lb)
{
	ParameterStruct tmp(*param);
	tmp.line_bottom = lb;
	set_from_struct(tmp);
}


bool ParagraphParameters::pagebreakTop() const
{
	return param->pagebreak_top;
}


void ParagraphParameters::pagebreakTop(bool pbt)
{
	ParameterStruct tmp(*param);
	tmp.pagebreak_top = pbt;
	set_from_struct(tmp);
}


bool ParagraphParameters::pagebreakBottom() const
{
	return param->pagebreak_bottom;
}


void ParagraphParameters::pagebreakBottom(bool pbb)
{
	ParameterStruct tmp(*param);
	tmp.pagebreak_bottom = pbb;
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
		} else if (token == "\\fill_top") {
			spaceTop(VSpace(VSpace::VFILL));
		} else if (token == "\\fill_bottom") {
			spaceBottom(VSpace(VSpace::VFILL));
		} else if (token == "\\line_top") {
			lineTop(true);
		} else if (token == "\\line_bottom") {
			lineBottom(true);
		} else if (token == "\\pagebreak_top") {
			pagebreakTop(true);
		} else if (token == "\\pagebreak_bottom") {
			pagebreakBottom(true);
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
		} else if (token == "\\added_space_top") {
			lex.nextToken();
			VSpace value = VSpace(lex.getString());
			// only add the length when value > 0 or
			// with option keep
			if ((value.length().len().value() != 0) ||
			    value.keep() ||
			    (value.kind() != VSpace::LENGTH))
				spaceTop(value);
		} else if (token == "\\added_space_bottom") {
			lex.nextToken();
			VSpace value = VSpace(lex.getString());
			// only add the length when value > 0 or
			// with option keep
			if ((value.length().len().value() != 0) ||
			   value.keep() ||
			    (value.kind() != VSpace::LENGTH))
				spaceBottom(value);
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
	// Maybe some vertical spaces.
	if (spaceTop().kind() != VSpace::NONE)
		os << "\\added_space_top "
		   << spaceTop().asLyXCommand() << ' ';
	if (spaceBottom().kind() != VSpace::NONE)
		os << "\\added_space_bottom "
		   << spaceBottom().asLyXCommand() << ' ';

	// Maybe the paragraph has special spacing
	spacing().writeFile(os, true);

	// The labelwidth string used in lists.
	if (!labelWidthString().empty())
		os << "\\labelwidthstring "
		   << labelWidthString() << '\n';

	// Lines above or below?
	if (lineTop())
		os << "\\line_top ";
	if (lineBottom())
		os << "\\line_bottom ";

	// Pagebreaks above or below?
	if (pagebreakTop())
		os << "\\pagebreak_top ";
	if (pagebreakBottom())
		os << "\\pagebreak_bottom ";

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
	istringstream is(STRCONV(data));
	LyXLex lex(0,0);
	lex.setStream(is);

	ParagraphParameters params;
	params.read(lex);

	LyXText * text = bv.getLyXText();
	text->setParagraph(params.lineTop(),
			   params.lineBottom(),
			   params.pagebreakTop(),
			   params.pagebreakBottom(),
			   params.spaceTop(),
			   params.spaceBottom(),
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

	data = STRCONV(os.str());
}
