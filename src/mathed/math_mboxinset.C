/**
 * \file math_mboxinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_mboxinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"

#include "BufferView.h"
#include "buffer.h"
#include "bufferparams.h"
#include "debug.h"
#include "metricsinfo.h"
#include "output_latex.h"
#include "outputparams.h"
#include "paragraph.h"
#include "texrow.h"

using std::auto_ptr;
using std::endl;


MathMBoxInset::MathMBoxInset(BufferView & bv)
	: text_(&bv), bv_(&bv)
{
	text_.paragraphs().clear();
	text_.paragraphs().push_back(Paragraph());
	text_.paragraphs().back().
		layout(bv.buffer()->params().getLyXTextClass().defaultLayout());
	text_.redoParagraph(0);
}


auto_ptr<InsetBase> MathMBoxInset::clone() const
{
	return auto_ptr<InsetBase>(new MathMBoxInset(*this));
}


void MathMBoxInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	text_.metrics(mi, dim);
	metricsMarkers2(dim);
	dim_ = dim;
}


void MathMBoxInset::draw(PainterInfo & pi, int x, int y) const
{
	text_.draw(pi, x + 1, y - text_.ascent());
	drawMarkers(pi, x, y);
}


void MathMBoxInset::write(WriteStream & ws) const
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


int MathMBoxInset::latex(Buffer const & buf, std::ostream & os,
			OutputParams const & runparams) const
{
	os << "\\mbox{\n";
	TexRow texrow;
	latexParagraphs(buf, text_.paragraphs(), os, texrow, runparams);
	os << "}";
	return texrow.rows();
}


void MathMBoxInset::priv_dispatch(LCursor & cur, FuncRequest & cmd)
{
	text_.dispatch(cur, cmd);
}


LyXText * MathMBoxInset::getText(int) const
{
	return &text_;
}


void MathMBoxInset::getCursorPos(CursorSlice const & cur, int & x, int & y) const
{
	x = text_.cursorX(cur);
	y = text_.cursorY(cur);
}
