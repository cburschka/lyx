/**
 * \file insetcaption.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetcaption.h"
#include "insetfloat.h"
#include "insetwrap.h"

#include "buffer.h"
#include "bufferparams.h"
#include "counters.h"
#include "cursor.h"
#include "BufferView.h"
#include "Floating.h"
#include "FloatList.h"
#include "gettext.h"
#include "LColor.h"
#include "metricsinfo.h"
#include "paragraph.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/lstrings.h"
#include "support/convert.h"

#include <sstream>


namespace lyx {

using support::bformat;

using std::auto_ptr;
using std::endl;
using std::string;
using std::ostream;


InsetCaption::InsetCaption(BufferParams const & bp)
	: InsetText(bp), textclass_(bp.getLyXTextClass())
{
	setAutoBreakRows(true);
	setDrawFrame(true);
	setFrameColor(LColor::captionframe);
}


void InsetCaption::write(Buffer const & buf, ostream & os) const
{
	os << "Caption\n";
	text_.write(buf, os);
}


void InsetCaption::read(Buffer const & buf, LyXLex & lex)
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


void InsetCaption::setLabel(LCursor & cur) const
{
	// Set caption label _only_ if the cursor is in _this_ float:
	if (cur.top().text() == &text_) {
		string s;
		size_t i = cur.depth();
			while (i > 0) {
				--i;
				InsetBase * const in = &cur[i].inset();
				if (in->lyxCode() == InsetBase::FLOAT_CODE
				    || in->lyxCode() == InsetBase::WRAP_CODE) {
					s = to_utf8(in->getInsetName());
					break;
				}
			}
		Floating const & fl = textclass_.floats().getType(s);
		s = fl.name();
		docstring num;
		if (s.empty())
			s = "Senseless";
		else
			num = convert<docstring>(counter_);

		// Generate the label
		label = to_utf8(
			bformat(from_ascii("%1$s %2$s:"), _(s), num));
	}
}


void InsetCaption::metrics(MetricsInfo & mi, Dimension & dim) const
{
	mi.base.textwidth -= 2 * TEXT_TO_INSET_OFFSET;
	LCursor cur = mi.base.bv->cursor();
	setLabel(cur);
	docstring dlab(label.begin(), label.end());
	labelwidth_ = theFontMetrics(mi.base.font).width(dlab);
	dim.wid = labelwidth_;
	Dimension textdim;
	InsetText::metrics(mi, textdim);
	dim.des = std::max(dim.des - textdim.asc + dim.asc, textdim.des);
	dim.asc = textdim.asc;
	dim.wid += textdim.wid;
	dim.asc += TEXT_TO_INSET_OFFSET;
	dim.des += TEXT_TO_INSET_OFFSET;
	dim.wid += 2 * TEXT_TO_INSET_OFFSET;
	mi.base.textwidth += 2 * TEXT_TO_INSET_OFFSET;
	dim_ = dim;
}


void InsetCaption::draw(PainterInfo & pi, int x, int y) const
{
	// We must draw the label, we should get the label string
	// from the enclosing float inset.
	// The question is: Who should draw the label, the caption inset,
	// the text inset or the paragraph?
	// We should also draw the float number (Lgb)

	// See if we can find the name of the float this caption
	// belongs to.
	LCursor cur = pi.base.bv->cursor();
	setLabel(cur);
	docstring dlab(label.begin(), label.end());
	// FXIME: instead of using the fontLoader metrics, we should make
	// painter::text() returns the drawn text witdh.
	labelwidth_ = theFontMetrics(pi.base.font).width(dlab);
	pi.pain.text(x, y, dlab, pi.base.font);
	InsetText::draw(pi, x + labelwidth_, y);
	setPosCache(pi, x, y);
}


void InsetCaption::edit(LCursor & cur, bool left)
{
	cur.push(*this);
	InsetText::edit(cur, left);
}


InsetBase * InsetCaption::editXY(LCursor & cur, int x, int y)
{
	cur.push(*this);
	return InsetText::editXY(cur, x, y);
}


int InsetCaption::latex(Buffer const & buf, odocstream & os,
			OutputParams const & runparams) const
{
	// This is a bit too simplistic to take advantage of
	// caption options we must add more later. (Lgb)
	// This code is currently only able to handle the simple
	// \caption{...}, later we will make it take advantage
	// of the one of the caption packages. (Lgb)
	odocstringstream ost;
	int const l = InsetText::latex(buf, ost, runparams);
	os << "\\caption{" << ost.str() << "}\n";
	return l + 1;
}


int InsetCaption::plaintext(Buffer const & /*buf*/, odocstream & /*os*/,
			OutputParams const & /*runparams*/) const
{
	// FIXME: Implement me!
	return 0;
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


auto_ptr<InsetBase> InsetCaption::doClone() const
{
	return auto_ptr<InsetBase>(new InsetCaption(*this));
}


} // namespace lyx
