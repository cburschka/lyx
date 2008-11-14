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
#include "MetricsInfo.h"
#include "output_latex.h"
#include "OutputParams.h"
#include "Paragraph.h"
#include "TexRow.h"
#include "TextMetrics.h"

#include "support/debug.h"

using namespace std;

namespace lyx {


InsetMathMBox::InsetMathMBox()
{
	text_.paragraphs().clear();
	text_.paragraphs().push_back(Paragraph());
}


InsetMathMBox::InsetMathMBox(Layout const & layout)
{
	text_.paragraphs().clear();
	text_.paragraphs().push_back(Paragraph());
	text_.paragraphs().back().setLayout(layout);
}


Inset * InsetMathMBox::clone() const
{
	return new InsetMathMBox(*this);
}


void InsetMathMBox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	TextMetrics & tm = mi.base.bv->textMetrics(&text_);
	tm.metrics(mi, dim);
	metricsMarkers2(dim);
}


void InsetMathMBox::draw(PainterInfo & pi, int x, int y) const
{
	pi.base.bv->textMetrics(&text_).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void InsetMathMBox::write(WriteStream & ws) const
{
	if (ws.latex()) {
		ws << "\\mbox{\n";
		TexRow texrow;
		OutputParams runparams(&buffer().params().encoding());
		latexParagraphs(buffer(), text_, ws.os(), texrow, runparams);
		ws.addlines(texrow.rows());
		ws << "}";
	} else {
		ws << "\\mbox{\n";
		ostringstream os;
		text_.write(buffer(), os);
		ws.os() << from_utf8(os.str());
		ws << "}";
	}
}


int InsetMathMBox::latex(odocstream & os, OutputParams const & runparams) const
{
	os << "\\mbox{\n";
	TexRow texrow;
	latexParagraphs(buffer(), text_, os, texrow, runparams);
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
	x = bv.textMetrics(&text_).cursorX(sl, boundary);
	y = bv.textMetrics(&text_).cursorY(sl, boundary);
}


} // namespace lyx
