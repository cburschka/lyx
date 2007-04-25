/**
 * \file Inset.cpp
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

#include "Inset.h"

#include "BufferView.h"
#include "debug.h"
#include "gettext.h"
#include "lyxtext.h"
#include "metricsinfo.h"
#include "coordcache.h"


namespace lyx {

using std::string;


InsetOld::InsetOld()
{}


InsetOld::InsetOld(InsetOld const & in)
	: InsetBase(in), name_(in.name_)
{}


void InsetOld::setPosCache(PainterInfo const & pi, int x, int y) const
{
	//lyxerr << "InsetOld:: position cache to " << x << " " << y << std::endl;
	pi.base.bv->coordCache().insets().add(this, x, y);
}


} // namespace lyx
