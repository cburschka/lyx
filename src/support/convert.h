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

#include "support/strfwd.h"

namespace lyx {

template <class Target, class Source>
Target convert(Source arg);


template<> std::string convert<std::string>(bool b);
template<> std::string convert<std::string>(char c);
template<> std::string convert<std::string>(short unsigned int sui);
template<> std::string convert<std::string>(int i);
template<> docstring convert<docstring>(int i);
template<> std::string convert<std::string>(unsigned int ui);
template<> docstring convert<docstring>(unsigned int ui);
template<> std::string convert<std::string>(unsigned long ul);
template<> docstring convert<docstring>(unsigned long ul);
template<> std::string convert<std::string>(long l);
template<> docstring convert<docstring>(long l);
template<> std::string convert<std::string>(float f);
template<> std::string convert<std::string>(double d);
template<> int convert<int>(std::string const & s);
template<> int convert<int>(docstring const & s);
template<> unsigned int convert<unsigned int>(std::string const & s);
template<> unsigned long convert<unsigned long>(std::string const & s);
template<> double convert<double>(std::string const & s);
template<> int convert<int>(char const * cptr);
template<> double convert<double>(char const * cptr);

} // namespace lyx

#endif
