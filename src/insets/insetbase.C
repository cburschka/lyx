/**
 * \file insetbase.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetbase.h"

#include "buffer.h"
#include "BufferView.h"
#include "LColor.h"
#include "cursor.h"
#include "debug.h"
#include "dimension.h"
#include "dispatchresult.h"
#include "gettext.h"
#include "lyxtext.h"
#include "metricsinfo.h"

#include "frontends/Painter.h"



void InsetBase::dispatch(LCursor & cur, FuncRequest const & cmd)
{
	priv_dispatch(cur, cmd);
}


void InsetBase::priv_dispatch(LCursor & cur, FuncRequest const &)
{
	cur.noupdate();
	cur.notdispatched();
}


bool InsetBase::getStatus(LCursor &, FuncRequest const &, FuncStatus &)
{
	return false;
}


void InsetBase::edit(LCursor &, bool)
{
	lyxerr << "InsetBase: edit left/right" << std::endl;
}


InsetBase * InsetBase::editXY(LCursor &, int x, int y)
{
	lyxerr << "InsetBase: editXY x:" << x << " y: " << y << std::endl;
	return this;
}


InsetBase::idx_type InsetBase::index(row_type row, col_type col) const
{
	if (row != 0)
		lyxerr << "illegal row: " << row << std::endl;
	if (col != 0)
		lyxerr << "illegal col: " << col << std::endl;
	return 0;
}


bool InsetBase::idxBetween(idx_type idx, idx_type from, idx_type to) const
{
	return from <= idx && idx <= to;
}


bool InsetBase::idxUpDown(LCursor &, bool) const
{
	return false;
}


bool InsetBase::idxUpDown2(LCursor &, bool) const
{
	return false;
}


int InsetBase::plaintext(Buffer const &,
	std::ostream &, OutputParams const &) const
{
	return 0;
}


int InsetBase::linuxdoc(Buffer const &,
	std::ostream &, OutputParams const &) const
{
	return 0;
}


int InsetBase::docbook(Buffer const &,
	std::ostream &, OutputParams const &) const
{
	return 0;
}


bool InsetBase::directWrite() const
{
	return false;
}


InsetBase::EDITABLE InsetBase::editable() const
{
	return NOT_EDITABLE;
}


bool InsetBase::autoDelete() const
{
	return false;
}


std::string const InsetBase::editMessage() const
{
	return _("Opened inset");
}


std::string const & InsetBase::getInsetName() const
{
	static std::string const name = "unknown";
	return name;
}


void InsetBase::markErased()
{}


void InsetBase::getCursorPos(CursorSlice const &, int & x, int & y) const
{
	lyxerr << "InsetBase::getCursorPos called directly" << std::endl;
	x = 100;
	y = 100;
}


void InsetBase::metricsMarkers(Dimension & dim, int) const
{
	dim.wid += 2;
	dim.asc += 1;
}


void InsetBase::metricsMarkers2(Dimension & dim, int) const
{
	dim.wid += 2;
	dim.asc += 1;
	dim.des += 1;
}


void InsetBase::drawMarkers(PainterInfo & pi, int x, int y) const
{
	if (!editing(pi.base.bv))
		return;
	int const t = x + width() - 1;
	int const d = y + descent();
	pi.pain.line(x, d - 3, x, d, LColor::mathframe);
	pi.pain.line(t, d - 3, t, d, LColor::mathframe);
	pi.pain.line(x, d, x + 3, d, LColor::mathframe);
	pi.pain.line(t - 3, d, t, d, LColor::mathframe);
	setPosCache(pi, x, y);
}


void InsetBase::drawMarkers2(PainterInfo & pi, int x, int y) const
{
	if (!editing(pi.base.bv))
		return;
	drawMarkers(pi, x, y);
	int const t = x + width() - 1;
	int const a = y - ascent();
	pi.pain.line(x, a + 3, x, a, LColor::mathframe);
	pi.pain.line(t, a + 3, t, a, LColor::mathframe);
	pi.pain.line(x, a, x + 3, a, LColor::mathframe);
	pi.pain.line(t - 3, a, t, a, LColor::mathframe);
	setPosCache(pi, x, y);
}


bool InsetBase::editing(BufferView * bv) const
{
	return bv->cursor().isInside(this);
}


bool InsetBase::covers(int x, int y) const
{
	return x >= xo()
			&& x <= xo() + width()
			&& y >= yo() - ascent()
			&& y <= yo() + descent();
}


void InsetBase::dump() const
{
	Buffer buf("foo", 1);
	write(buf, lyxerr);
}


/////////////////////////////////////////

bool isEditableInset(InsetBase const * inset)
{
	return inset && inset->editable();
}


bool isHighlyEditableInset(InsetBase const * inset)
{
	return inset && inset->editable() == InsetBase::HIGHLY_EDITABLE;
}


