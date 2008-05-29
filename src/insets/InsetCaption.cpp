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
#include "support/gettext.h"
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

using namespace std;
using namespace lyx::support;

namespace lyx {


InsetCaption::InsetCaption(Buffer const & buf)
	: InsetText(buf)
{
	setAutoBreakRows(true);
	setDrawFrame(true);
	setFrameColor(Color_captionframe);
	// There will always be only one
	paragraphs().back().setLayout(buf.params().documentClass().emptyLayout());
}


void InsetCaption::write(ostream & os) const
{
	os << "Caption\n";
	text_.write(buffer(), os);
}


void InsetCaption::read(Lexer & lex)
{
#if 0
	// We will enably this check again when the compability
	// code is removed from Buffer::Read (Lgb)
	lex.setContext("InsetCaption::Read: consistency check");
	lex >> "Caption";
#endif
	InsetText::read(lex);
}


docstring InsetCaption::editMessage() const
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
	if (!isAscii(label) || label.empty())
		// This must be a user defined layout. We cannot translate
		// this, since gettext accepts only ascii keys.
		custom_label_ = label;
	else
		custom_label_ = _(to_ascii(label));
}


void InsetCaption::addToToc(DocIterator const & cpit)
{
	if (type_.empty())
		return;

	DocIterator pit = cpit;
	pit.push_back(CursorSlice(*this));

	Toc & toc = buffer().tocBackend().toc(type_);
	docstring const str = full_label_ + ". " + text_.getPar(0).asString();
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
	dim.des = max(dim.des - textdim.asc + dim.asc, textdim.des);
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


void InsetCaption::edit(Cursor & cur, bool front, EntryDirection entry_from)
{
	cur.push(*this);
	InsetText::edit(cur, front, entry_from);
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
	case MATHMACRO_CODE:
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
		status.setEnabled(false);
		return true;

	case LFUN_OPTIONAL_INSERT:
		status.setEnabled(cur.paragraph().insetList().find(OPTARG_CODE) == -1);
		return true;

	case LFUN_INSET_TOGGLE:
		// pass back to owner
		cur.undispatched();
		return false;

	default:
		return InsetText::getStatus(cur, cmd, status);
	}
}


int InsetCaption::latex(odocstream & os,
			OutputParams const & runparams_in) const
{
	if (in_subfloat_)
		// caption is output as an optional argument
		return 0;
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
	int l = latexOptArgInsets(paragraphs()[0], os, runparams, 1);
	os << '{';
	l += InsetText::latex(os, runparams);
	os << "}\n";
	runparams_in.encoding = runparams.encoding;
	return l + 1;
}


int InsetCaption::plaintext(odocstream & os,
			    OutputParams const & runparams) const
{
	os << '[' << full_label_ << "\n";
	InsetText::plaintext(os, runparams);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


int InsetCaption::docbook(odocstream & os,
			  OutputParams const & runparams) const
{
	int ret;
	os << "<title>";
	ret = InsetText::docbook(os, runparams);
	os << "</title>\n";
	return ret;
}


int InsetCaption::getArgument(odocstream & os,
			OutputParams const & runparams) const
{
	return InsetText::latex(os, runparams);
}


int InsetCaption::getOptArg(odocstream & os,
			OutputParams const & runparams) const
{
	return latexOptArgInsets(paragraphs()[0], os, runparams, 1);
}


int InsetCaption::getCaptionText(odocstream & os,
			OutputParams const & runparams) const
{
	os << full_label_ << ' ';
	return InsetText::plaintext(os, runparams);
}


void InsetCaption::updateLabels(ParIterator const & it)
{
	Buffer const & master = *buffer().masterBuffer();
	DocumentClass const & tclass = master.params().documentClass();
	Counters & cnts = tclass.counters();
	string const & type = cnts.current_float();
	// Memorize type for addToToc().
	type_ = type;
	in_subfloat_ = cnts.isSubfloat();
	if (type.empty())
		full_label_ = master.B_("Senseless!!! ");
	else {
		// FIXME: life would be _much_ simpler if listings was
		// listed in Floating.
		docstring name;
		if (type == "listing")
			name = master.B_("Listing");
		else
			name = master.B_(tclass.floats().getType(type).name());
		docstring counter = from_utf8(type);
		if (in_subfloat_) {
			counter = "sub-" + from_utf8(type);
			name = bformat(_("Sub-%1$s"),
				       master.B_(tclass.floats().getType(type).name()));
		}
		if (cnts.hasCounter(counter)) {
			cnts.step(counter);
			full_label_ = bformat(from_ascii("%1$s %2$s:"), 
					      name,
					      cnts.theCounter(counter));
		} else
			full_label_ = bformat(from_ascii("%1$s #:"), name);	
	}

	// Do the real work now.
	InsetText::updateLabels(it);
}


} // namespace lyx
