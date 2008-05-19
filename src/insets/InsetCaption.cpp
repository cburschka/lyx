/**
 * \file InsetCaption.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetCaption.h"
#include "InsetFloat.h"
#include "InsetWrap.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Counters.h"
#include "Cursor.h"
#include "BufferView.h"
#include "Floating.h"
#include "FloatList.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "Color.h"
#include "MetricsInfo.h"
#include "output_latex.h"
#include "OutputParams.h"
#include "Paragraph.h"
#include "ParIterator.h"
#include "paragraph_funcs.h"
#include "TocBackend.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/lstrings.h"
#include "support/convert.h"

#include <sstream>


using std::auto_ptr;
using std::endl;
using std::string;
using std::ostream;


namespace lyx {

using support::bformat;

InsetCaption::InsetCaption(InsetCaption const & ic)
	: InsetText(ic), textclass_(ic.textclass_)
{
	setAutoBreakRows(true);
	setDrawFrame(true);
	setFrameColor(Color::captionframe);
}

InsetCaption::InsetCaption(BufferParams const & bp)
	: InsetText(bp), textclass_(bp.getTextClass())
{
	setAutoBreakRows(true);
	setDrawFrame(true);
	setFrameColor(Color::captionframe);
}


void InsetCaption::write(Buffer const & buf, ostream & os) const
{
	os << "Caption\n";
	text_.write(buf, os);
}


void InsetCaption::read(Buffer const & buf, Lexer & lex)
{
#if 0
	// We will enably this check again when the compability
	// code is removed from Buffer::Read (Lgb)
	string const token = lex.GetString();
	if (token != "Caption") {
		lyxerr << "InsetCaption::Read: consistency check failed."
		       << endl;
	}
#endif
	InsetText::read(buf, lex);
}


docstring const InsetCaption::editMessage() const
{
	return _("Opened Caption Inset");
}


void InsetCaption::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const
{
	InsetText::cursorPos(bv, sl, boundary, x, y);
	x += labelwidth_;
}


void InsetCaption::setCustomLabel(docstring const & label)
{
	if (!support::isAscii(label) || label.empty())
		// This must be a user defined layout. We cannot translate
		// this, since gettext accepts only ascii keys.
		custom_label_ = label;
	else
		custom_label_ = _(to_ascii(label));
}


void InsetCaption::addToToc(TocList & toclist, Buffer const & buf,
	ParConstIterator const & cpit) const
{
	if (type_.empty())
		return;

	ParConstIterator pit = cpit;
	pit.push_back(CursorSlice(const_cast<InsetCaption &>(*this)));

	Toc & toc = toclist[type_];
	docstring const str = convert<docstring>(counter_)
		+ ". " + pit->asString(buf, false);
	toc.push_back(TocItem(pit, 0, str));
}


bool InsetCaption::metrics(MetricsInfo & mi, Dimension & dim) const
{
	int const width_offset = TEXT_TO_INSET_OFFSET / 2;
	mi.base.textwidth -= width_offset;

	computeFullLabel(*mi.base.bv->buffer());

	Font tmpfont = mi.base.font;
	mi.base.font = mi.base.bv->buffer()->params().getFont();
	labelwidth_ = theFontMetrics(mi.base.font).width(full_label_);
	// add some space to separate the label from the inset text
	labelwidth_ += 2 * TEXT_TO_INSET_OFFSET;
	dim.wid = labelwidth_;
	Dimension textdim;
	InsetText::metrics(mi, textdim);
	// Correct for button width, and re-fit
	mi.base.textwidth -= dim.wid;
	InsetText::metrics(mi, textdim);
	mi.base.font = tmpfont;
	dim.des = std::max(dim.des - textdim.asc + dim.asc, textdim.des);
	dim.asc = textdim.asc;
	dim.wid += textdim.wid;
	dim.asc += TEXT_TO_INSET_OFFSET;
	dim.des += TEXT_TO_INSET_OFFSET;
	dim.wid += width_offset;
	mi.base.textwidth += width_offset;
	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


void InsetCaption::draw(PainterInfo & pi, int x, int y) const
{
	// We must draw the label, we should get the label string
	// from the enclosing float inset.
	// The question is: Who should draw the label, the caption inset,
	// the text inset or the paragraph?
	// We should also draw the float number (Lgb)

	// Answer: the text inset (in buffer_funcs.cpp: setCaption).

	Font tmpfont = pi.base.font;
	pi.base.font = pi.base.bv->buffer()->params().getFont();
	labelwidth_ = pi.pain.text(x, y, full_label_, pi.base.font);
	// add some space to separate the label from the inset text
	labelwidth_ += 2 * TEXT_TO_INSET_OFFSET;
	InsetText::draw(pi, x + labelwidth_, y);
	setPosCache(pi, x, y);
	pi.base.font = tmpfont;
}


void InsetCaption::drawSelection(PainterInfo & pi, int x, int y) const
{
	InsetText::drawSelection(pi, x + labelwidth_, y);
}


void InsetCaption::edit(Cursor & cur, bool left)
{
	cur.push(*this);
	InsetText::edit(cur, left);
}


Inset * InsetCaption::editXY(Cursor & cur, int x, int y)
{
	cur.push(*this);
	return InsetText::editXY(cur, x, y);
}


bool InsetCaption::insetAllowed(Inset::Code code) const
{
	switch (code) {
	case FLOAT_CODE:
	case TABULAR_CODE:
	case WRAP_CODE:
	case CAPTION_CODE:
	case PAGEBREAK_CODE:
		return false;
	default:
		return InsetText::insetAllowed(code);
	}
}


bool InsetCaption::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {

	case LFUN_BREAK_PARAGRAPH:
	case LFUN_BREAK_PARAGRAPH_KEEP_LAYOUT:
	case LFUN_BREAK_PARAGRAPH_SKIP:
		status.enabled(false);
		return true;

	case LFUN_OPTIONAL_INSERT:
		status.enabled(numberOfOptArgs(cur.paragraph()) == 0);
		return true;

	default:
		return InsetText::getStatus(cur, cmd, status);
	}
}


int InsetCaption::latex(Buffer const & buf, odocstream & os,
			OutputParams const & runparams_in) const
{
	// This is a bit too simplistic to take advantage of
	// caption options we must add more later. (Lgb)
	// This code is currently only able to handle the simple
	// \caption{...}, later we will make it take advantage
	// of the one of the caption packages. (Lgb)
	OutputParams runparams = runparams_in;
	// FIXME: actually, it is moving only when there is no
	// optional argument.
	runparams.moving_arg = true;
	os << "\\caption";
	int l = latexOptArgInsets(buf, paragraphs()[0], os, runparams, 1);
	os << '{';
	l += InsetText::latex(buf, os, runparams);
	os << "}\n";
	runparams_in.encoding = runparams.encoding;
	return l + 1;
}


int InsetCaption::plaintext(Buffer const & buf, odocstream & os,
			    OutputParams const & runparams) const
{
	computeFullLabel(buf);

	os << '[' << full_label_ << "\n";
	InsetText::plaintext(buf, os, runparams);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


int InsetCaption::docbook(Buffer const & buf, odocstream & os,
			  OutputParams const & runparams) const
{
	int ret;
	os << "<title>";
	ret = InsetText::docbook(buf, os, runparams);
	os << "</title>\n";
	return ret;
}


int InsetCaption::getArgument(Buffer const & buf, odocstream & os,
			OutputParams const & runparams) const
{
	return InsetText::latex(buf, os, runparams);
}


int InsetCaption::getOptArg(Buffer const & buf, odocstream & os,
			OutputParams const & runparams) const
{
	return latexOptArgInsets(buf, paragraphs()[0], os, runparams, 1);
}


void InsetCaption::computeFullLabel(Buffer const & buf) const
{
	if (type_.empty())
		full_label_ = buf.B_("Senseless!!! ");
	else {
		docstring const number = convert<docstring>(counter_);
		docstring label = custom_label_.empty()? buf.B_(type_): custom_label_;
		full_label_ = bformat(from_ascii("%1$s %2$s:"), label, number);
	}
}


auto_ptr<Inset> InsetCaption::doClone() const
{
	return auto_ptr<Inset>(new InsetCaption(*this));
}


} // namespace lyx
