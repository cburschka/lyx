/**
 * \file InsetLine.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetLine.h"

#include "Buffer.h"
#include "Dimension.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "Length.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "output_xhtml.h"
#include "Text.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

using namespace std;

namespace lyx {

using frontend::Painter;


InsetLine::InsetLine(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p, "line")
{}


ParamInfo const & InsetLine::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("offset", ParamInfo::LYX_INTERNAL);
		param_info_.add("width", ParamInfo::LYX_INTERNAL);
		param_info_.add("height", ParamInfo::LYX_INTERNAL);
	}
	return param_info_;
}


docstring InsetLine::screenLabel() const
{
	return _("Horizontal line");
}


void InsetLine::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p(LINE_CODE);
		// FIXME UNICODE
		InsetCommand::string2params("line",
			to_utf8(cmd.argument()), p);
		if (p.getCmdName().empty()) {
			cur.noScreenUpdate();
			break;
		}
		setParams(p);
		break;
	}

	default:
		InsetCommand::doDispatch(cur, cmd);
		break;
	}
}


bool InsetLine::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action()) {
	case LFUN_INSET_DIALOG_UPDATE:
	case LFUN_INSET_MODIFY:
		status.setEnabled(true);
		return true;
	default:
		return InsetCommand::getStatus(cur, cmd, status);
	}
}


void InsetLine::metrics(MetricsInfo & mi, Dimension & dim) const
{
	frontend::FontMetrics const & fm = theFontMetrics(mi.base.font);
	dim.asc = fm.maxAscent();
	dim.des = fm.maxDescent();

	Length const width(to_ascii(getParam("width")));
	int w = width.inPixels(mi.base.textwidth,
			       fm.width(char_type('M')));

	// assure that the line inset is not outside of the window
	// check that it doesn't exceed the outer boundary
	if (w > mi.base.textwidth)
		w = mi.base.textwidth;

	// set a minimal width
	int const minw = (w < 0) ? 24 : 4;
	dim.wid = max(minw, max(w, -w));

	// Cache the inset dimension
	setDimCache(mi, dim);
}


void InsetLine::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);

	frontend::FontMetrics const & fm = theFontMetrics(pi.base.font);

	// get the length of the parameters in pixels
	Length offset = Length(to_ascii(getParam("offset")));
	int o = offset.inPixels(pi.base.textwidth,
				fm.width(char_type('M')));
	Length const width(to_ascii(getParam("width")));
	int w = width.inPixels(pi.base.textwidth,
		fm.width(char_type('M')));
	Length height = Length(to_ascii(getParam("height")));
	int h = height.inPixels(pi.base.textwidth,
		fm.width(char_type('M')));

	// get the surrounding text color
	FontInfo f = pi.base.font;
	Color Line_color = f.realColor();

	// assure that the drawn line is not outside of the window
	// check that it doesn't exceed the outer boundary
	if (x + w - h/2 - 2 > pi.base.textwidth)
		w = pi.base.textwidth - x + h/2 + 2;
	// check that it doesn't exceed the upper boundary
	if (y - o - h/2 < 0)
		o = y - h/2 - 2;

	// the offset is a vertical one
	pi.pain.line(x + h/2 + 1, y - o - h/2, x + w - h/2 - 2, y - o - h/2,
		Line_color, Painter::line_solid, float(h));
}


int InsetLine::latex(odocstream & os, OutputParams const &) const
{
	bool have_offset = true;
	Length offset_len = Length(to_ascii(getParam("offset")));
	if (offset_len.value() == 0)
		have_offset = false;

	string const offset =
		Length(to_ascii(getParam("offset"))).asLatexString();
	string const width =
		Length(to_ascii(getParam("width"))).asLatexString();
	string const height =
		Length(to_ascii(getParam("height"))).asLatexString();

	os << "\\rule";
	// only output the optional parameter if the offset is not 0
	if (have_offset)
		os	<< "[" << from_ascii(offset) << "]";
	os << "{" << from_ascii(width) << "}{" << from_ascii(height) << '}';

	return 0;
}


int InsetLine::plaintext(odocstream & os, OutputParams const &) const
{
	os << "\n-------------------------------------------\n";
	return PLAINTEXT_NEWLINE;
}


int InsetLine::docbook(odocstream & os, OutputParams const &) const
{
	os << '\n';
	return 0;
}


docstring InsetLine::xhtml(XHTMLStream & xs, OutputParams const &) const
{
	xs << html::CompTag("hr");
	xs.cr();
	return docstring();
}


} // namespace lyx
