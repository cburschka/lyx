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
#include "BufferView.h"
#include "Counters.h"
#include "Cursor.h"
#include "Dimension.h"
#include "Floating.h"
#include "FloatList.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "InsetList.h"
#include "MetricsInfo.h"
#include "output_latex.h"
#include "OutputParams.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/lstrings.h"

#include <sstream>


using std::endl;
using std::string;
using std::ostream;


namespace lyx {


InsetCaption::InsetCaption(InsetCaption const & ic)
	: InsetText(ic), textclass_(ic.textclass_)
{
	setAutoBreakRows(true);
	setDrawFrame(true);
	setFrameColor(Color_captionframe);
}

InsetCaption::InsetCaption(BufferParams const & bp)
	: InsetText(bp), textclass_(bp.getTextClass())
{
	setAutoBreakRows(true);
	setDrawFrame(true);
	setFrameColor(Color_captionframe);
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


void InsetCaption::addToToc(TocList & toclist, Buffer const & buf, ParConstIterator const &) const
{
	if (type_.empty())
		return;

	ParConstIterator pit = par_const_iterator_begin(*this);

	Toc & toc = toclist[type_];
	docstring const str = full_label_ + ". " + pit->asString(buf, false);
	toc.push_back(TocItem(pit, 0, str));
}


void InsetCaption::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontInfo tmpfont = mi.base.font;
	mi.base.font = mi.base.bv->buffer().params().getFont().fontInfo();
	labelwidth_ = theFontMetrics(mi.base.font).width(full_label_);
	// add some space to separate the label from the inset text
	labelwidth_ += 2 * TEXT_TO_INSET_OFFSET;
	dim.wid = labelwidth_;
	Dimension textdim;
	// Correct for button and label width
	mi.base.textwidth -= dim.wid;
	InsetText::metrics(mi, textdim);
	mi.base.font = tmpfont;
	mi.base.textwidth += dim.wid;
	dim.des = std::max(dim.des - textdim.asc + dim.asc, textdim.des);
	dim.asc = textdim.asc;
	dim.wid += textdim.wid;
}


void InsetCaption::draw(PainterInfo & pi, int x, int y) const
{
	// We must draw the label, we should get the label string
	// from the enclosing float inset.
	// The question is: Who should draw the label, the caption inset,
	// the text inset or the paragraph?
	// We should also draw the float number (Lgb)

	// Answer: the text inset (in buffer_funcs.cpp: setCaption).

	FontInfo tmpfont = pi.base.font;
	pi.base.font = pi.base.bv->buffer().params().getFont().fontInfo();
	pi.pain.text(x, y, full_label_, pi.base.font);
	InsetText::draw(pi, x + labelwidth_, y);
	pi.base.font = tmpfont;
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


bool InsetCaption::insetAllowed(InsetCode code) const
{
	switch (code) {
	case FLOAT_CODE:
	case TABULAR_CODE:
	case WRAP_CODE:
	case CAPTION_CODE:
	case NEWPAGE_CODE:
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
	case LFUN_BREAK_PARAGRAPH_SKIP:
		status.enabled(false);
		return true;

	case LFUN_OPTIONAL_INSERT:
		status.enabled(cur.paragraph().insetList().find(OPTARG_CODE) == -1);
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


void InsetCaption::updateLabels(Buffer const & buf, ParIterator const & it)
{
	using support::bformat;
	TextClass const & tclass = buf.params().getTextClass();
	Counters & cnts = tclass.counters();
	string const & type = cnts.current_float();
	if (type.empty())
		full_label_ = buf.B_("Senseless!!! ");
	else {
		// FIXME: life would be _much_ simpler if listings was
		// listed in Floating.
		docstring name;
		if (type == "listing")
			name = buf.B_("Listing");
		else
			name = buf.B_(tclass.floats().getType(type).name());
		if (cnts.hasCounter(from_utf8(type))) {
			cnts.step(from_utf8(type));
			full_label_ = bformat(from_ascii("%1$s %2$s:"), 
					      name, 
					      cnts.theCounter(from_utf8(type)));
		} else
			full_label_ = bformat(from_ascii("%1$s #:"), name);	
	}

	// Do the real work now.
	InsetText::updateLabels(buf, it);
}


Inset * InsetCaption::clone() const
{
	return new InsetCaption(*this);
}


} // namespace lyx
