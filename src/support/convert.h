// -*- C++ -*-
/**
 * \file convert.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 *
 * A collection of string helper functions that works with string.
 * Some of these would certainly benefit from a rewrite/optimization.
 */

#ifndef CONVERT_H
#define CONVERT_H

#if 0
#include <boost/static_assert.hpp>

// Commented out since BOOST_STATIC_ASSERT does not work with gcc 4.0
template <class Target, class Source>
Target convert(Source arg)
{
	// We use a static assert here since we want all instances of
	// this template to be specializations.
	BOOST_STATIC_ASSERT(sizeof(bool) == 0);
	return Target();
}
#else
template <class Target, class Source>
Target convert(Source arg);
#endif

#endif
