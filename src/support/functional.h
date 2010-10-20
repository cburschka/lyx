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

#ifdef LYX_USE_TR1

#include <functional>

#ifdef __GNUC__
#include <tr1/functional>
#endif

namespace lyx
{
	using std::tr1::function;
}

#else

#include <boost/function.hpp>
#include <boost/functional.hpp>

namespace lyx
{
	using boost::function;
}

#endif


#endif
