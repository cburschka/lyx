/**
 * \file ControlFloat.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "ControlFloat.h"
#include "BufferView.h"
#include "buffer.h"


ControlFloat::ControlFloat(LyXView & lv, Dialogs & d)
	: ControlInset<InsetFloat, InsetFloatParams>(lv, d)
{}


void ControlFloat::applyParamsToInset()
{
	inset()->placement(params().placement);
	inset()->wide(params().wide, bufferview()->buffer()->params);
	bufferview()->updateInset(inset(), true);

}


void ControlFloat::applyParamsNoInset()
{}


InsetFloatParams const ControlFloat::getParams(InsetFloat const & inset)
{
	return inset.params();
}
