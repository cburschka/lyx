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
#include "debug.h"
#include "dispatchresult.h"
#include "gettext.h"
#include "lyxtext.h"


DispatchResult InsetBase::dispatch(LCursor & cur, FuncRequest const & cmd)
{
	return priv_dispatch(cur, cmd);
}


DispatchResult InsetBase::priv_dispatch(LCursor &, FuncRequest const &)
{
	return DispatchResult(false);
}


void InsetBase::edit(LCursor &, bool)
{
	lyxerr << "InsetBase: edit left/right" << std::endl;
}


void InsetBase::edit(LCursor & cur, int, int)
{
	lyxerr << "InsetBase: edit xy" << std::endl;
	edit(cur, true);
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


void InsetBase::getScreenPos(idx_type, pos_type, int & x, int & y) const
{
	lyxerr << "InsetBase::getScreenPos() called directly!" << std::endl;
	x = y = 0;
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


bool InsetBase::insetAllowed(InsetBase * inset) const
{
	return insetAllowed(inset->lyxCode());
}


std::string const & InsetBase::getInsetName() const
{
	static std::string const name = "unknown";
	return name;
}


int InsetBase::getCell(int x, int y) const
{
	for (int i = 0, n = numParagraphs(); i < n; ++i) {
		LyXText * text = getText(i);
		//lyxerr << "### text: " << text << " i: " << i
		//	<< " xo: " << text->xo_ << "..." << text->xo_ + text->width
		//	<< " yo: " << text->yo_ 
		//	<< " yo: " << text->yo_ - text->ascent() << "..."
		//		<<  text->yo_ + text->descent()
		//	<< std::endl;	
		if (x >= text->xo_
				&& x <= text->xo_ + text->width
				&& y >= text->yo_ 
				&& y <= text->yo_ + text->height)
		{
			lyxerr << "### found text # " << i << std::endl;	
			return i;
		}
	}
	return -1;
}


void InsetBase::markErased()
{}

/////////////////////////////////////////

bool isEditableInset(InsetBase const * i)
{
	return i && i->editable();
}


bool isHighlyEditableInset(InsetBase const * i)
{
	return i && i->editable() == InsetBase::HIGHLY_EDITABLE;
}


