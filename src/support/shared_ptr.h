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

#ifdef LYX_USE_CXX11

#include <memory>
#define LYX_SHAREDPTR_NS std

#else

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#define LYX_SHAREDPTR_NS boost

#endif

namespace lyx
{
	using LYX_SHAREDPTR_NS::shared_ptr;
	using LYX_SHAREDPTR_NS::make_shared;
	using LYX_SHAREDPTR_NS::const_pointer_cast;
}

#undef LYX_SHAREDPTR_NS


#endif
