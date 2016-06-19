/**
 * \file InsetVSpace.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author various
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetVSpace.h"

#include "Buffer.h"
#include "BufferView.h"
#include "Cursor.h"
#include "Dimension.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "output_xhtml.h"
#include "texstream.h"
#include "Text.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lassert.h"

#include "frontends/Application.h"
#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include <sstream>

using namespace std;

namespace lyx {

namespace {

int const ADD_TO_VSPACE_WIDTH = 5;

} // namespace anon


InsetVSpace::InsetVSpace(VSpace const & space)
	: Inset(0), space_(space)
{}


void InsetVSpace::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		cur.recordUndo();
		string arg = to_utf8(cmd.argument());
		if (arg == "vspace custom")
			arg = (space_.kind() == VSpace::LENGTH)
				? "vspace " + space_.length().asString()
				: "vspace 1" + string(stringFromUnit(Length::defaultUnit()));
		InsetVSpace::string2params(arg, space_);
		break;
	}

	default:
		Inset::doDispatch(cur, cmd);
		break;
	}
}


bool InsetVSpace::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action()) {
	// we handle these
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "vspace") {
			VSpace vspace;
			string arg = to_utf8(cmd.argument());
			if (arg == "vspace custom")
				arg = (space_.kind() == VSpace::LENGTH)
				? "vspace " + space_.length().asString()
				: "vspace 1" + string(stringFromUnit(Length::defaultUnit()));
			InsetVSpace::string2params(arg, vspace);
			status.setOnOff(vspace == space_);
		} 
		status.setEnabled(true);
		return true;
	
	default:
		return Inset::getStatus(cur, cmd, status);
	}
}


void InsetVSpace::read(Lexer & lex)
{
	LASSERT(lex.isOK(), return);
	string vsp;
	lex >> vsp;
	if (lex)
		space_ = VSpace(vsp);
	lex >> "\\end_inset";
}


void InsetVSpace::write(ostream & os) const
{
	os << "VSpace " << space_.asLyXCommand();
}


docstring const InsetVSpace::label() const
{
	static docstring const label = _("Vertical Space");
	return label + " (" + space_.asGUIName() + ')';
}


namespace {
int const vspace_arrow_size = 4;
}


void InsetVSpace::metrics(MetricsInfo & mi, Dimension & dim) const
{
	int height = 3 * vspace_arrow_size;
	if (space_.length().len().value() >= 0.0)
		height = max(height, space_.inPixels(*mi.base.bv));

	FontInfo font;
	font.decSize();
	font.decSize();

	int w = 0;
	int a = 0;
	int d = 0;
	theFontMetrics(font).rectText(label(), w, a, d);

	height = max(height, a + d);

	dim.asc = height / 2 + (a - d) / 2; // align cursor with the
	dim.des = height - dim.asc;         // label text
	dim.wid = ADD_TO_VSPACE_WIDTH + 2 * vspace_arrow_size + 5 + w;
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetVSpace::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	x += ADD_TO_VSPACE_WIDTH;
	int const start = y - dim.asc;
	int const end   = y + dim.des;

	// y-values for top arrow
	int ty1, ty2;
	// y-values for bottom arrow
	int by1, by2;

	if (space_.kind() == VSpace::VFILL) {
		ty1 = ty2 = start;
		by1 = by2 = end;
	} else {
		// adding or removing space
		bool const added = space_.kind() != VSpace::LENGTH ||
				   space_.length().len().value() >= 0.0;
		ty1 = added ? (start + vspace_arrow_size) : start;
		ty2 = added ? start : (start + vspace_arrow_size);
		by1 = added ? (end - vspace_arrow_size) : end;
		by2 = added ? end : (end - vspace_arrow_size);
	}

	int const midx = x + vspace_arrow_size;
	int const rightx = midx + vspace_arrow_size;

	// first the string
	int w = 0;
	int a = 0;
	int d = 0;

	FontInfo font;
	font.setColor(Color_added_space);
	font.decSize();
	font.decSize();
	docstring const lab = label();
	theFontMetrics(font).rectText(lab, w, a, d);

	pi.pain.rectText(x + 2 * vspace_arrow_size + 5,
			 start + (end - start) / 2 + (a - d) / 2,
			 lab, font, Color_none, Color_none);

	// top arrow
	pi.pain.line(x, ty1, midx, ty2, Color_added_space);
	pi.pain.line(midx, ty2, rightx, ty1, Color_added_space);

	// bottom arrow
	pi.pain.line(x, by1, midx, by2, Color_added_space);
	pi.pain.line(midx, by2, rightx, by1, Color_added_space);

	// joining line
	pi.pain.line(midx, ty2, midx, by2, Color_added_space);
}


void InsetVSpace::latex(otexstream & os, OutputParams const &) const
{
	os << from_ascii(space_.asLatexCommand(buffer().params())) << '\n';
}


int InsetVSpace::plaintext(odocstringstream & os,
        OutputParams const &, size_t) const
{
	os << "\n\n";
	return PLAINTEXT_NEWLINE;
}


int InsetVSpace::docbook(odocstream & os, OutputParams const &) const
{
	os << '\n';
	return 1;
}


docstring InsetVSpace::xhtml(XHTMLStream & os, OutputParams const &) const
{
	string const len = space_.asHTMLLength();
	string const attr = "style='height:" + (len.empty() ? "1em" : len) + "'";
	os << html::StartTag("div", attr, true) << html::EndTag("div");
	return docstring();
}


string InsetVSpace::contextMenuName() const
{
	return "context-vspace";
}


void InsetVSpace::string2params(string const & in, VSpace & vspace)
{
	vspace = VSpace();
	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex;
	lex.setStream(data);
	lex.setContext("InsetVSpace::string2params");
	lex >> "vspace" >> vspace;
}


string InsetVSpace::params2string(VSpace const & vspace)
{
	ostringstream data;
	data << "vspace" << ' ' << vspace.asLyXCommand();
	return data.str();
}


} // namespace lyx
