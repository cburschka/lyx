// -*- C++ -*-
/**
 * \file tostr.h
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

#ifndef TOSTR_H
#define TOSTR_H

#include <boost/static_assert.hpp>

#include <string>

template <class Target, class Source>
Target convert(Source arg)
{
	// We use a static assert here since we want all instances of
	// this template to be specializations.
	BOOST_STATIC_ASSERT(sizeof(bool) == 0);
	return Target();
}

template<>
std::string convert<std::string>(bool);

template<>
std::string convert<std::string>(char);

template<>
std::string convert<std::string>(unsigned short);

template<>
std::string convert<std::string>(int);

template<>
std::string convert<std::string>(unsigned int);

template<>
std::string convert<std::string>(float);

template<>
std::string convert<std::string>(double);

template<>
std::string convert<std::string>(std::string);

#endif
