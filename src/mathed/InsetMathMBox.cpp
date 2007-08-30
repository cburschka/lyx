/**
 * \file InsetMathMBox.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathMBox.h"
#include "MathData.h"
#include "MathStream.h"

#include "BufferView.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "Cursor.h"
#include "debug.h"
#include "MetricsInfo.h"
#include "output_latex.h"
#include "OutputParams.h"
#include "Paragraph.h"
#include "TexRow.h"
#include "TextMetrics.h"


namespace lyx {

using std::endl;


InsetMathMBox::InsetMathMBox()
{
	text_.paragraphs().clear();
	text_.paragraphs().push_back(Paragraph());
}


InsetMathMBox::InsetMathMBox(LayoutPtr const & layout)
{
	text_.paragraphs().clear();
	text_.paragraphs().push_back(Paragraph());
	text_.paragraphs().back().layout(layout);
}


Inset * InsetMathMBox::clone() const
{
	return new InsetMathMBox(*this);
}


bool InsetMathMBox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	TextMetrics & tm = mi.base.bv->textMetrics(&text_);
	tm.metrics(mi, dim);
	metricsMarkers2(dim);
	if (dim_ == dim)
		return false;
	dim_ = dim;
	return true;
}


void InsetMathMBox::draw(PainterInfo & pi, int x, int y) const
{
	text_.draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void InsetMathMBox::write(Buffer const & buf, WriteStream & ws) const
{
	if (ws.latex()) {
		ws << "\\mbox{\n";
		TexRow texrow;
		OutputParams runparams(&buf.params().encoding());
		latexParagraphs(buf, text_.paragraphs(), ws.os(), texrow, runparams);
		ws.addlines(texrow.rows());
		ws << "}";
	} else {
		ws << "\\mbox{\n";
		std::ostringstream os;
		text_.write(buf, os);
		ws.os() << from_utf8(os.str());
		ws << "}";
	}
}


int InsetMathMBox::latex(Buffer const & buf, odocstream & os,
			OutputParams const & runparams) const
{
	os << "\\mbox{\n";
	TexRow texrow;
	latexParagraphs(buf, text_.paragraphs(), os, texrow, runparams);
	os << "}";
	return texrow.rows();
}


void InsetMathMBox::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	text_.dispatch(cur, cmd);
}


Text * InsetMathMBox::getText(int) const
{
	return &text_;
}


void InsetMathMBox::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const
{
	x = text_.cursorX(bv, sl, boundary);
	y = text_.cursorY(bv, sl, boundary);
}


void InsetMathMBox::drawSelection(PainterInfo & pi, int x, int y) const
{
	text_.drawSelection(pi, x, y);
}


} // namespace lyx
