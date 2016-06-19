/**
 * \file InsetLine.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 * \author André Pönitz
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetLine.h"

#include "Buffer.h"
#include "Cursor.h"
#include "Dimension.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "Length.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "output_xhtml.h"
#include "texstream.h"
#include "Text.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <cstdlib>

using namespace std;

namespace lyx {

using frontend::Painter;


InsetLine::InsetLine(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p), height_(0), offset_(0)
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
		InsetCommand::string2params(to_utf8(cmd.argument()), p);
		if (p.getCmdName().empty()) {
			cur.noScreenUpdate();
			break;
		}
		cur.recordUndo();
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
	int const max_width = mi.base.textwidth;

	Length const width(to_ascii(getParam("width")));
	dim.wid = width.inPixels(mi.base);

	// assure that the line inset is not outside of the window
	// check that it doesn't exceed the outer boundary
	if (dim.wid > max_width)
		dim.wid = max_width;

	// set a minimal width
	int const minw = (dim.wid < 0) ? 24 : 4;
	dim.wid = max(minw, abs(dim.wid));

	Length height = Length(to_ascii(getParam("height")));
	height_ = height.inPixels(mi.base);

	// get the length of the parameters in pixels
	Length offset = Length(to_ascii(getParam("offset")));
	offset_ = offset.inPixels(mi.base);

	dim.asc = max(fm.maxAscent(), offset_ + height_);
	dim.des = max(fm.maxDescent(), - offset_);

	// Cache the inset dimension
	setDimCache(mi, dim);
}


Dimension const InsetLine::dimension(BufferView const & bv) const
{
	// We cannot use InsetCommand::dimension() as this returns the dimension
	// of the button, which is not used here.
	return Inset::dimension(bv);
}


void InsetLine::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);

	// get the surrounding text color
	Color Line_color = pi.base.font.realColor();

	// the offset is a vertical one
	// the horizontal dimension must be corrected with the heigth because
	// of left and right border of the painted line for big heigth.
	pi.pain.line(x + height_/2 + 1,
		     y - offset_ - height_/2,
		     x + dim.wid - height_/2 - 2,
		     y - offset_ - height_/2,
		     Line_color, Painter::line_solid, height_);
}


void InsetLine::latex(otexstream & os, OutputParams const &) const
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
}


int InsetLine::plaintext(odocstringstream & os,
        OutputParams const &, size_t) const
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
	xs << html::CompTag("hr") << html::CR();
	return docstring();
}


} // namespace lyx
