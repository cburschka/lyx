// -*- C++ -*-
/**
 * \file functional.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_FUNCTIONAL_H
#define LYX_FUNCTIONAL_H

#if __cplusplus >= 201103L

#include <functional>
#define LYX_FUNCTIONAL_NS std

#else

#include <boost/function.hpp>
#include <boost/functional.hpp>
#define LYX_FUNCTIONAL_NS boost

#endif

namespace lyx
{
	using LYX_FUNCTIONAL_NS::function;
}

#undef LYX_FUNCTIONAL_NS


#endif
