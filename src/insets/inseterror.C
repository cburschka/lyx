/**
 * \file inseterror.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "inseterror.h"

#include "BufferView.h"
#include "dimension.h"
#include "funcrequest.h"
#include "gettext.h"
#include "lyxfont.h"
#include "metricsinfo.h"

#include "frontends/Dialogs.h"
#include "frontends/font_metrics.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"

#include "support/LAssert.h"

using std::ostream;


InsetError::InsetError(string const & str)
	: contents(str)
{}


// InsetError::InsetError(string const & str, bool)
//	: contents(str)
// {}


InsetError::~InsetError()
{
	Dialogs::hide("error", this);
}


dispatch_result InsetError::localDispatch(FuncRequest const & cmd)
{
	// UNUSED: dispatch_result result = UNDISPATCHED;

	switch (cmd.action) {
	case LFUN_MOUSE_RELEASE:
	case LFUN_INSET_EDIT:
		cmd.view()->owner()->getDialogs().show("error", getContents(), this);
		return DISPATCHED;

	default:
		return Inset::localDispatch(cmd);
	}
}


void InsetError::metrics(MetricsInfo & mi, Dimension & dim) const
{
	LyXFont efont;
	efont.setSize(mi.base.font.size()).decSize();
	dim_.asc = font_metrics::maxAscent(efont) + 1;
	dim_.des = font_metrics::maxDescent(efont) + 1;
	dim_.wid = 6 + font_metrics::width(_("Error"), efont);
	dim = dim_;
}


void InsetError::draw(PainterInfo & pi, int x, int y) const
{
	lyx::Assert(pi.base.bv);
	cache(pi.base.bv);

	LyXFont efont;
	efont.setSize(pi.base.font.size()).decSize();
	efont.setColor(LColor::error);

	// Draw as "Error" in a framed box
	x += 1;
	Dimension dim;
	MetricsInfo mi;
	mi.base.bv = pi.base.bv;
	mi.base.font = pi.base.font;
	metrics(mi, dim);
	dim_ = dim;
	pi.pain.fillRectangle(x, y - dim.asc + 1,
	      dim.wid - 2, dim.asc + dim.des - 2, LColor::insetbg);
	pi.pain.rectangle(x, y - dim.asc + 1,
	      dim.wid - 2, dim.asc + dim.des - 2, LColor::error);
	pi.pain.text(x + 2, y, _("Error"), efont);
}


string const InsetError::editMessage() const
{
	return _("Opened error");
}
