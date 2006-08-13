// -*- C++ -*-
/**
 * \file docstring.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Provide a set of typedefs for commonly used things like sizes and
 * indices wile trying to stay compatible with types used
 * by the standard containers.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_DOCSTRING_H
#define LYX_DOCSTRING_H

#include <boost/cstdint.hpp>
#include <string>

namespace lyx {

typedef std::basic_string<boost::uint32_t> docstring;

}

#endif
