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
#include "BufferView.h"
#include "Floating.h"
#include "FloatList.h"
#include "gettext.h"
#include "metricsinfo.h"

#include "frontends/Painter.h"
#include "frontends/font_metrics.h"

#include "support/LAssert.h"
#include "support/lstrings.h"

#include "Lsstream.h"

using namespace lyx::support;

using std::endl;
using std::ostream;


InsetCaption::InsetCaption(BufferParams const & bp)
	: InsetText(bp)
{
	setAutoBreakRows(true);
	setDrawFrame(InsetText::LOCKED);
	setFrameColor(LColor::captionframe);
}


void InsetCaption::write(Buffer const & buf, ostream & os) const
{
	os << "Caption\n";
	writeParagraphData(buf, os);
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


string const InsetCaption::editMessage() const
{
	return _("Opened Caption Inset");
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
	InsetOld * i1 = owner();
	InsetOld * i2 = i1 ? i1->owner() : 0;
	string type;
	if (i2->lyxCode() == FLOAT_CODE)
		type = static_cast<InsetFloat *>(i2)->params().type;
	else if (i2->lyxCode() == WRAP_CODE)
		type = static_cast<InsetWrap *>(i2)->params().type;
	else
		Assert(0);

	FloatList const & floats =
		pi.base.bv->buffer()->params.getLyXTextClass().floats();
	string const fl = i2 ? floats.getType(type).name() : N_("Float");

	// Discover the number...
	string const num = "#";

	// Generate the label
	string const label = bformat("%1$s %2$s:", _(fl), num);
	int const w = font_metrics::width(label, pi.base.font);
	pi.pain.text(x, y, label, pi.base.font);
	InsetText::draw(pi, x + w, y);
}


int InsetCaption::latex(Buffer const & buf, ostream & os,
			LatexRunParams const & runparams) const
{
	// This is a bit too simplistic to take advantage of
	// caption options we must add more later. (Lgb)
	// This code is currently only able to handle the simple
	// \caption{...}, later we will make it take advantage
	// of the one of the caption packages. (Lgb)
	ostringstream ost;
	int const l = InsetText::latex(buf, ost, runparams);
	os << "\\caption{" << ost.str() << "}\n";
	return l + 1;
}


int InsetCaption::ascii(Buffer const & /*buf*/,
			ostream & /*os*/, int /*linelen*/) const
{
	// FIX: Implement me!
	return 0;
}


int InsetCaption::docbook(Buffer const & buf, ostream & os, bool mixcont) const
{
	int ret;
	os << "<title>";
	ret = InsetText::docbook(buf, os, mixcont);
	os << "</title>\n";
	return ret;
}
