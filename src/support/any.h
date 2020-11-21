// -*- C++ -*-
/**
 * \file any.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Yuriy Skalko
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_ANY_H
#define LYX_ANY_H

#if __cplusplus >= 201703L

#include <any>

namespace lyx {
using std::any;
using std::any_cast;
}

#else

#include <boost/any.hpp>

namespace lyx {
using boost::any;
using boost::any_cast;
}

#endif // __cplusplus >= 201703L

#endif // LYX_ANY_H
