/**
 * \file updatableinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "updatableinset.h"

#include "BufferView.h"
#include "coordcache.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "lyxtext.h"

#include "support/convert.h"

#include <boost/assert.hpp>


// An updatable inset is highly editable by definition
InsetBase::EDITABLE UpdatableInset::editable() const
{
	return HIGHLY_EDITABLE;
}


int UpdatableInset::scroll(bool) const
{
	return scx;
}


void UpdatableInset::setScroll(int maxwidth, int pos) const
{
	scx = pos;
}


void UpdatableInset::getCursorDim(int &, int &) const
{
	BOOST_ASSERT(false);
}

