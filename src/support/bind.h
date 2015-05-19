// -*- C++ -*-
/**
 * \file bind.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_BIND_H
#define LYX_BIND_H

#include "support/functional.h"

#ifdef LYX_USE_CXX11

#define LYX_BIND_NS std

namespace lyx
{
	using std::placeholders::_1;
	using std::placeholders::_2;
}

#else

#include <boost/bind.hpp>
#define LYX_BIND_NS boost

#endif

namespace lyx
{
	using LYX_BIND_NS::bind;
	using LYX_BIND_NS::ref;
}

#undef LYX_BIND_NS


#endif
