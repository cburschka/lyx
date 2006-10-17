/**
 * \file InsetMathMBox.C
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
#include "MathMLStream.h"

#include "BufferView.h"
#include "buffer.h"
#include "bufferparams.h"
#include "cursor.h"
#include "debug.h"
#include "metricsinfo.h"
#include "output_latex.h"
#include "outputparams.h"
#include "paragraph.h"
#include "texrow.h"

using std::auto_ptr;
using std::endl;


InsetMathMBox::InsetMathMBox(BufferView & bv)
	: text_(&bv), bv_(&bv)
{
	text_.paragraphs().clear();
	text_.paragraphs().push_back(Paragraph());
	text_.paragraphs().back().
		layout(bv.buffer()->params().getLyXTextClass().defaultLayout());
	text_.redoParagraph(0);
}


auto_ptr<InsetBase> InsetMathMBox::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathMBox(*this));
}


void InsetMathMBox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	text_.metrics(mi, dim);
	metricsMarkers2(dim);
	dim_ = dim;
}


void InsetMathMBox::draw(PainterInfo & pi, int x, int y) const
{
	text_.draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void InsetMathMBox::write(WriteStream & ws) const
{
	if (ws.latex()) {
		ws << "\\mbox{\n";
		TexRow texrow;
		OutputParams runparams;
		latexParagraphs(*bv_->buffer(), text_.paragraphs(),
			ws.os(), texrow, runparams);
		ws.addlines(texrow.rows());
		ws << "}";
	} else {
		ws << "\\mbox{\n";
		text_.write(*bv_->buffer(), ws.os());
		ws << "}";
	}
}


int InsetMathMBox::latex(Buffer const & buf, std::ostream & os,
			OutputParams const & runparams) const
{
	os << "\\mbox{\n";
	TexRow texrow;
	latexParagraphs(buf, text_.paragraphs(), os, texrow, runparams);
	os << "}";
	return texrow.rows();
}


void InsetMathMBox::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	text_.dispatch(cur, cmd);
}


LyXText * InsetMathMBox::getText(int) const
{
	return &text_;
}


void InsetMathMBox::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const
{
	x = text_.cursorX(sl, boundary);
	y = text_.cursorY(sl, boundary);
}


void InsetMathMBox::drawSelection(PainterInfo & pi, int x, int y) const
{
	text_.drawSelection(pi, x, y);
}
