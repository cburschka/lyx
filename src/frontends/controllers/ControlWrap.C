/**
 * \file ControlWrap.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlWrap.h"
#include "BufferView.h"
#include "buffer.h"


ControlWrap::ControlWrap(LyXView & lv, Dialogs & d)
	: ControlInset<InsetWrap, WrapParams>(lv, d)
{}



void ControlWrap::applyParamsToInset()
{
	inset()->pageWidth(params().pageWidth);
	inset()->placement(params().placement);
	bufferview()->updateInset(inset(), true);

}


void ControlWrap::applyParamsNoInset()
{}


WrapParams const ControlWrap::getParams(InsetWrap const & inset)
{
	return WrapParams(inset);
}


WrapParams::WrapParams(InsetWrap const & inset)
	: pageWidth(inset.pageWidth()),
	  placement(inset.placement())
{}
