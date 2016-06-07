// -*- C++ -*-
/**
 * \file shared_ptr.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_SHARED_PTR_H
#define LYX_SHARED_PTR_H

#include <memory>

namespace lyx
{
	using std::shared_ptr;
	using std::make_shared;
	using std::const_pointer_cast;
}


#endif
