/**
 * \file render_base.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "render_base.h"

#include "BufferView.h"


BufferView * RenderBase::view() const
{
	return view_.lock().get();
}
